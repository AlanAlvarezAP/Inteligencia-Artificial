#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <random>
#include <mutex>
#include <cmath>
#include <ctime>
#include <atomic>
#include <fstream>
#include <string>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int GRAPH_HEIGHT = 600;
const unsigned int CHART_HEIGHT = 350;
const unsigned int SCR_HEIGHT = GRAPH_HEIGHT + CHART_HEIGHT;
const int NUM_NODES = 10;

// shaders

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource_edges = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);\n"
    "}\n\0";
	
const char *fragmentShaderSource_nodes = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.1f, 0.6f, 0.9f, 1.0f);\n"
    "}\n\0";

const char *fragmentShaderSource_route = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.85f, 0.1f, 1.0f);\n"
    "}\n\0";

const char *fragmentShaderSource_chartBest = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.2f, 1.0f, 0.4f, 1.0f);\n"
    "}\n\0";

const char *fragmentShaderSource_chartAvg = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.1f, 1.0f);\n"
    "}\n\0";

const char *fragmentShaderSource_axes = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.7f, 0.7f, 0.7f, 1.0f);\n"
    "}\n\0";

// ===========================================================

// EASY FONT
 
// txt shaders
const char *textVertSrc = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform vec2 uResolution;\n"
    "uniform vec2 uOffset;\n"
    "void main() {\n"
    "    vec2 p = (aPos + uOffset) / uResolution * 2.0 - 1.0;\n"
    "    p.y = -p.y;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}\0";
 
const char *textFragSrc = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    FragColor = vec4(uColor, 1.0);\n"
    "}\0";
 
class EasyFontRenderer {
public:
    unsigned int VAO = 0, VBO = 0, prog = 0;
 
    void init()
	{
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // space for text
        glBufferData(GL_ARRAY_BUFFER, 1024 * 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        // use for xy only
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
 
        auto comp = [](GLenum t, const char* src)
		{
            unsigned int s = glCreateShader(t);
            glShaderSource(s, 1, &src, NULL);
            glCompileShader(s);
            return s;
        };
        unsigned int vs = comp(GL_VERTEX_SHADER,   textVertSrc);
        unsigned int fs = comp(GL_FRAGMENT_SHADER, textFragSrc);
        prog = glCreateProgram();
        glAttachShader(prog, vs);
		glAttachShader(prog, fs);
        glLinkProgram(prog);
        glDeleteShader(vs);
		glDeleteShader(fs);
    }
 
    // x y in viewport pixels
    // vpW vpH - viewport dimensions
    void draw(const char* text, float x, float y, float scale,
		float r, float g, float b, float vpW, float vpH)
    {
        static char buf[1024 * 16];
        int numQuads = stb_easy_font_print(0, 0, (char*)text, NULL, buf, sizeof(buf));
 
        // conver quads to triangles triángulos - 2 per quad = 6 vertex
        struct V4 { float x, y, z, w; };
        V4* verts = (V4*)buf;
 
        vector<float> tris;
        tris.reserve(numQuads * 6 * 2);
        for (int q = 0; q < numQuads; q++)
		{
            V4* v = verts + q * 4;
            // tri 1
            tris.push_back(v[0].x * scale); tris.push_back(v[0].y * scale); tris.push_back(0); tris.push_back(0);
            tris.push_back(v[1].x * scale); tris.push_back(v[1].y * scale); tris.push_back(0); tris.push_back(0);
            tris.push_back(v[2].x * scale); tris.push_back(v[2].y * scale); tris.push_back(0); tris.push_back(0);
            // tri 2
            tris.push_back(v[0].x * scale); tris.push_back(v[0].y * scale); tris.push_back(0); tris.push_back(0);
            tris.push_back(v[2].x * scale); tris.push_back(v[2].y * scale); tris.push_back(0); tris.push_back(0);
            tris.push_back(v[3].x * scale); tris.push_back(v[3].y * scale); tris.push_back(0); tris.push_back(0);
        }
 
        glUseProgram(prog);
        glUniform2f(glGetUniformLocation(prog, "uResolution"), vpW, vpH);
        glUniform2f(glGetUniformLocation(prog, "uOffset"), x, y);
        glUniform3f(glGetUniformLocation(prog, "uColor"), r, g, b);
 
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, tris.size()*sizeof(float), tris.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)tris.size() / 4);
        glBindVertexArray(0);
    }
 
    void cleanup()
	{
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(prog);
    }
};

// ===========================================================

// STRUCTS

struct Point
{
	float x, y;
};

struct DrawBatch
{
	int start, count;
};

// GRAPH

class Graph
{
private:
    int n;
    vector<Point> pts;
    vector<vector<float>> dist;
 
public:
    Graph() {}
	
    Graph(const vector<Point>& centers)
		: n((int)centers.size()), pts(centers)
	{
        dist.resize(n, vector<float>(n, 0.0f));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                float dx = pts[i].x - pts[j].x;
                float dy = pts[i].y - pts[j].y;
                dist[i][j] = sqrt(dx*dx + dy*dy);
            }
    }
    float getDistance(int u, int v) const
	{
		return dist[u][v];
	}
    int size() const
	{
		return n;
	}
    const Point& getPoint(int i) const
	{
		return pts[i];
	}
};

// GENETIC ALGORITHM

class GeneticAlgorithm
{
private:
    Graph& graph;
    int populationSize;
    double mutationRate;
 
    using Chromosome = vector<int>;
    vector<Chromosome> population;
    vector<mt19937> engines;

public:
    mutex          bestMutex;
    Chromosome     bestChromosome;
    float          bestFitness = 1e9f;
    atomic<bool>   running{true};
 
    // for graphic
    vector<float> historyBest;
    vector<float> historyAvg;

    GeneticAlgorithm(Graph& g, int popSize, double mutRate)
        : graph(g), populationSize(popSize), mutationRate(mutRate)
    {
        random_device rd;
        for (int i = 0; i < populationSize; ++i)
            engines.emplace_back(rd());
    }
 
    float fitness(const Chromosome& c)
	{
        float total = 0.0f;
        for (int i = 0; i < (int)c.size() - 1; i++)
            total += graph.getDistance(c[i], c[i+1]);
        total += graph.getDistance(c.back(), c[0]);
        return total;
    }

    void tournamentTask(int start, int end, vector<Chromosome>& nextPop, int engineIdx)
	{
        uniform_int_distribution<int> dist(0, populationSize - 1);
        for (int k = start; k < end; k++)
		{
            int i = dist(engines[engineIdx]);
            int j = dist(engines[engineIdx]);
            nextPop[k] = (fitness(population[i]) < fitness(population[j]))
				? population[i] : population[j];
        }
    }

    void crossoverRangeTask(int start, int end, vector<Chromosome>& nextPop, int engineIdx)
	{
        int n = graph.size();
        uniform_int_distribution<int> distPos(0, n - 1);
 
        for (int k = start; k < end - 1; k += 2)
		{
            int s = distPos(engines[engineIdx]);
            int e = distPos(engines[engineIdx]);
            if (s > e)
				swap(s, e);
 
            Chromosome p1 = nextPop[k], p2 = nextPop[k+1];
            Chromosome c1(n, -1), c2(n, -1);
 
            for (int i = s; i <= e; i++)
			{
				c1[i] = p1[i];
				c2[i] = p2[i];
			}
 
            auto fill = [&](Chromosome& child, const Chromosome& parent)
			{
                int curr = (e + 1) % n;
                for (int i = 0; i < n; i++)
				{
                    int gene = parent[(e + 1 + i) % n];
                    bool exists = false;
                    for (int g : child)
						if (g == gene)
						{
							exists = true;
							break;
						}
                    if (!exists)
					{
						child[curr] = gene;
						curr = (curr + 1) % n;
					}
                }
            };
            fill(c1, p2);
			fill(c2, p1);
            nextPop[k] = c1;
			nextPop[k+1] = c2;
        }
    }
 
    void mutationTask(int start, int end, vector<Chromosome>& nextPop, int engineIdx)
	{
        uniform_real_distribution<double> prob(0.0, 1.0);
        uniform_int_distribution<int> distGene(0, graph.size() - 1);
        for (int k = start; k < end; k++)
		{
            if (prob(engines[engineIdx]) < mutationRate)
                swap(nextPop[k][distGene(engines[engineIdx])],
					nextPop[k][distGene(engines[engineIdx])]);
        }
    }

    void run(int patience)
	{
        int n = graph.size();

        for (int i = 0; i < populationSize; i++) {
            Chromosome c(n);
            iota(c.begin(), c.end(), 0);
            shuffle(c.begin(), c.end(), engines[0]);
            population.push_back(c);
        }

        int gen = 0, stayCount = 0;
        int numThreads = min((int)thread::hardware_concurrency(), 8);
        int chunkSize = max(1, populationSize / numThreads);
 
        while (stayCount < patience && running)
		{
            vector<Chromosome> nextPop(populationSize);
            vector<thread> threads;
 
            // tourtnament
            for (int i = 0; i < numThreads; i++)
			{
                int s = i * chunkSize;
                int e = (i == numThreads-1) ? populationSize : s + chunkSize;
                if (s < e)
					threads.emplace_back(&GeneticAlgorithm::tournamentTask, this, s, e, ref(nextPop), i);
            }
            for (auto& t : threads)
				t.join();
			threads.clear();
 
            // crossover
            for (int i = 0; i < numThreads; i++)
			{
                int s = i * chunkSize; if (s % 2 != 0) s--;
                int e = (i == numThreads-1) ? populationSize : s + chunkSize;
                if (s < e)
					threads.emplace_back(&GeneticAlgorithm::crossoverRangeTask, this, s, e, ref(nextPop), i);
            }
            for (auto& t : threads)
				t.join();
			threads.clear();
 
            // mutation
            for (int i = 0; i < numThreads; i++)
			{
                int s = i * chunkSize;
                int e = (i == numThreads-1) ? populationSize : s + chunkSize;
                if (s < e)
					threads.emplace_back(&GeneticAlgorithm::mutationTask, this, s, e, ref(nextPop), i);
            }
            for (auto& t : threads)
				t.join();
			threads.clear();
 
            // selection
            Chromosome bestInGen = population[0];
            float minFit = fitness(population[0]);
			float sumFit = 0.0f;
            for (auto& c : population)
			{
                float f = fitness(c);
				sumFit += f;
                if (f < minFit)
				{
					minFit = f;
					bestInGen = c;
				}
            }
			float avgFit = sumFit / populationSize;
 
			historyAvg.push_back(avgFit);
			historyBest.push_back(minFit);
			lock_guard<mutex> lock(bestMutex);
 
            if (minFit < bestFitness)
			{
                bestFitness = minFit;
                bestChromosome = bestInGen;
				stayCount   = 0;
            }
			else
                stayCount++;
 
            population = move(nextPop);
			// elitism
            //population[0] = bestInGen;
 
            cout << "Gen: " << gen << " - Mejor dist: " << bestFitness 
				<< " Estanc: " << stayCount << endl;
            gen++;
        }
		cout << "\nAG terminado\n";
		/*
        cout << "Mejor distancia: " << bestFitness << endl;
        cout << "Ruta: ";
        {
            lock_guard<mutex> lock(bestMutex);
            for (int x : bestChromosome)
				cout << char('A' + x) << " ";
        }
		*/
        cout << endl;
    }
};

// ===========================================================

// HELPERS OpenGL
 
unsigned int compileShader(GLenum type, const char* src)
{
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    return s;
}
 
unsigned int linkProgram(unsigned int vs, unsigned int fs)
{
    unsigned int p = glCreateProgram();
    glAttachShader(p, vs);
	glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
	glDeleteShader(fs);
    return p;
}

// ===========================================================

int main()
{
	srand(time(NULL));
	
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TSP - Algoritmo Genetico", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
	
	// easy font
    EasyFontRenderer txt;
    txt.init();

    // build and compile our shader program
    // ------------------------------------
	
	unsigned int vs_shared = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	
	auto makeVS = [&]()
	{
		return compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	};
 
    unsigned int prog_edges = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_edges));
    unsigned int prog_nodes = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_nodes));
    unsigned int prog_route = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_route));
    unsigned int prog_chartBest = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_chartBest));
    unsigned int prog_chartAvg = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_chartAvg));
	unsigned int prog_axes = linkProgram(makeVS(), compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource_axes));
	
	// set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

	// var settings
	float radius = 0.04f;
	float margin = 1.0f - radius;
	int segments = 20;
	
	// generate random points
    std::vector<Point> centers(NUM_NODES);
	
    for (int i = 0; i < NUM_NODES; i++)
	{
        centers[i].x = ((float)rand() / RAND_MAX) * (2.0f * margin) - margin;
        centers[i].y = ((float)rand() / RAND_MAX) * (2.0f * margin) - margin;
    }
	
	// create graph
	Graph graph(centers);
	
	// prepare genetic algorithm
	
	// inputs
	int inPopSize, inPatience;
	float inMutationRate;
	
	cout<<"\nIngresar datos:\nPoblacion: ";
	cin>>inPopSize;
	cout<<"Indice de mutacion: ";
	cin>>inMutationRate;
	cout<<"Limite de estancamiento: ";
	cin>>inPatience;
	
	// create GA
    GeneticAlgorithm ga(graph, inPopSize, inMutationRate);
    thread gaThread([&]()
	{
		ga.run(inPatience);
	});
	
	// geometry
	vector<float> staticVerts;
    
    // edges - all conected
    for (int i = 0; i < NUM_NODES; i++)
	{
        for (int j = i + 1; j < NUM_NODES; j++)
		{
            staticVerts.push_back(centers[i].x);
			staticVerts.push_back(centers[i].y);
			staticVerts.push_back(0.0f);
            staticVerts.push_back(centers[j].x);
			staticVerts.push_back(centers[j].y);
			staticVerts.push_back(0.0f);
        }
    }
	
	DrawBatch edgesBatch;
    edgesBatch.start = 0;
    edgesBatch.count = (int)(staticVerts.size() / 3);

	std::vector<DrawBatch> nodosBatches;
	
	// nodes - circles
    for (int i = 0; i < NUM_NODES; i++)
	{
        DrawBatch batch;
        batch.start = (int)(staticVerts.size() / 3);
        // center
        staticVerts.push_back(centers[i].x); 
        staticVerts.push_back(centers[i].y); 
        staticVerts.push_back(0.0f);

        for (int s = 0; s <= segments; s++)
		{
            float angle = s * 2.0f * M_PI / segments;
            staticVerts.push_back(centers[i].x + cos(angle) * radius);
            staticVerts.push_back(centers[i].y + sin(angle) * radius);
            staticVerts.push_back(0.0f);
        }
        batch.count = (int)(staticVerts.size() / 3) - batch.start;
        nodosBatches.push_back(batch);
    }
	
	// axis
	vector<float> axesVerts = {
        // y
        -0.9f, -0.9f, 0.0f,
        -0.9f,  0.9f, 0.0f,
        // x
        -0.9f, -0.9f, 0.0f,
         0.9f, -0.9f, 0.0f,
    };

    int axesVertCount = (int)(axesVerts.size() / 3);
	
	// VAOs - VBOs
	// for graph
    unsigned int VAO_static, VBO_static;
    glGenVertexArrays(1, &VAO_static);
    glGenBuffers(1, &VBO_static);
    glBindVertexArray(VAO_static);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_static);
	glBufferData(GL_ARRAY_BUFFER, staticVerts.size() * sizeof(float), staticVerts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); 
	
	// for best route TSP
	unsigned int VAO_route, VBO_route;
    glGenVertexArrays(1, &VAO_route);
    glGenBuffers(1, &VBO_route);
    glBindVertexArray(VAO_route);
	
    glBindBuffer(GL_ARRAY_BUFFER, VBO_route);
    glBufferData(GL_ARRAY_BUFFER, (NUM_NODES + 1) * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
	
	// for graphic fitness vs generation
	unsigned int VAO_chartBest, VBO_chartBest;
    glGenVertexArrays(1, &VAO_chartBest);
    glGenBuffers(1, &VBO_chartBest);
    glBindVertexArray(VAO_chartBest);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_chartBest);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
 
    unsigned int VAO_chartAvg, VBO_chartAvg;
    glGenVertexArrays(1, &VAO_chartAvg);
    glGenBuffers(1, &VBO_chartAvg);
    glBindVertexArray(VAO_chartAvg);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_chartAvg);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
	
	// for axis
	unsigned int VAO_axes, VBO_axes;
    glGenVertexArrays(1, &VAO_axes);
    glGenBuffers(1, &VBO_axes);
    glBindVertexArray(VAO_axes);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
    glBufferData(GL_ARRAY_BUFFER, axesVerts.size()*sizeof(float), axesVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	vector<int> lastRoute;
    int routeVertCount = 0;
	
	// history local snapshot
    vector<float> localBest, localAvg;
    int chartBestCount = 0, chartAvgCount = 0;

    // normalize x y values
    auto buildChartVerts = [](const vector<float>& data, float minVal, float maxVal)
	{
        vector<float> verts;
        int n = (int)data.size();
        float range = maxVal - minVal;
        if (range < 1e-6f) range = 1.0f;
        for (int i = 0; i < n; i++)
		{
            float x = (n == 1) ? 0.0f : -0.9f + 1.8f * i / (n - 1);
			float y = -0.9f + 1.8f * (data[i] - minVal) / range;
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(0.0f);
        }
        return verts;
    };

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
		
		// rute update
        {
            lock_guard<mutex> lock(ga.bestMutex);
            if (ga.bestChromosome != lastRoute && !ga.bestChromosome.empty())
			{
                lastRoute = ga.bestChromosome;
                // make vertices
                vector<float> routeVerts;
                for (int idx : lastRoute)
				{
                    routeVerts.push_back(centers[idx].x);
                    routeVerts.push_back(centers[idx].y);
                    routeVerts.push_back(0.0f);
                }
                // close cicle
                routeVerts.push_back(centers[lastRoute[0]].x);
                routeVerts.push_back(centers[lastRoute[0]].y);
                routeVerts.push_back(0.0f);
 
                routeVertCount = (int)(routeVerts.size() / 3);
 
                glBindBuffer(GL_ARRAY_BUFFER, VBO_route);
                glBufferSubData(GL_ARRAY_BUFFER, 0, routeVerts.size() * sizeof(float), routeVerts.data());
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
		// graphic update
        {
            lock_guard<mutex> lock(ga.bestMutex);
            if ((int)ga.historyBest.size() > (int)localBest.size())
			{
                localBest = ga.historyBest;
                localAvg  = ga.historyAvg;
            }
        }

        if ((int)localBest.size() > chartBestCount)
		{
            chartBestCount = (int)localBest.size();
            chartAvgCount = (int)localAvg.size();
            // fitness global range
            float minVal = *min_element(localBest.begin(), localBest.end());
            float maxVal = *max_element(localAvg.begin(), localAvg.end());
 
            auto vBest = buildChartVerts(localBest, minVal, maxVal);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_chartBest);
            glBufferData(GL_ARRAY_BUFFER, vBest.size()*sizeof(float), vBest.data(), GL_DYNAMIC_DRAW);
 
            auto vAvg = buildChartVerts(localAvg, minVal, maxVal);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_chartAvg);
            glBufferData(GL_ARRAY_BUFFER, vAvg.size()*sizeof(float), vAvg.data(), GL_DYNAMIC_DRAW);
 
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		//glBindVertexArray(VAO);
		
		// drawing		
		// SUPERIOR VIEW
        glViewport(0, CHART_HEIGHT, SCR_WIDTH, GRAPH_HEIGHT);
		
        glUseProgram(prog_edges);
        glBindVertexArray(VAO_static);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, edgesBatch.start, edgesBatch.count);
 
        if (routeVertCount > 1)
		{
            glUseProgram(prog_route);
            glBindVertexArray(VAO_route);
            glLineWidth(3.0f);
            glDrawArrays(GL_LINE_STRIP, 0, routeVertCount);
        }
 
        glUseProgram(prog_nodes);
        glBindVertexArray(VAO_static);
        for (const auto& batch : nodosBatches)
            glDrawArrays(GL_TRIANGLE_FAN, batch.start, batch.count);
		
        // INFERIOR VIEW
        glViewport(0, 0, SCR_WIDTH, CHART_HEIGHT);
		
        glUseProgram(prog_axes);
        glBindVertexArray(VAO_axes);
        glLineWidth(1.5f);
        glDrawArrays(GL_LINES, 0, axesVertCount);
 
        if (chartBestCount > 1)
		{
            glLineWidth(2.0f);
 
            glUseProgram(prog_chartBest);
            glBindVertexArray(VAO_chartBest);
            glDrawArrays(GL_LINE_STRIP, 0, chartBestCount);
 
            glUseProgram(prog_chartAvg);
            glBindVertexArray(VAO_chartAvg);
            glDrawArrays(GL_LINE_STRIP, 0, chartAvgCount);
        }
		
        txt.draw("Generacion", SCR_WIDTH/2.0f - 40.0f, CHART_HEIGHT - 18.0f,
			1.5f, 0.8f, 0.8f, 0.8f, (float)SCR_WIDTH, (float)CHART_HEIGHT);
        txt.draw("Fitness", 6.0f, 14.0f,
			1.5f, 0.8f, 0.8f, 0.8f, (float)SCR_WIDTH, (float)CHART_HEIGHT);
        txt.draw("-- Mejor", SCR_WIDTH - 115.0f, 14.0f,
			1.5f, 0.2f, 1.0f, 0.4f, (float)SCR_WIDTH, (float)CHART_HEIGHT);
        txt.draw("-- Promedio", SCR_WIDTH - 115.0f, 32.0f,
			1.5f, 1.0f, 0.5f, 0.1f, (float)SCR_WIDTH, (float)CHART_HEIGHT);
		
        // glBindVertexArray(0); // no need to unbind it every time 
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	
	// stop GA and wait
    ga.running = false;
    gaThread.join();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO_static);
    glDeleteBuffers(1, &VBO_static);
    glDeleteVertexArrays(1, &VAO_route);
    glDeleteBuffers(1, &VBO_route);
    glDeleteVertexArrays(1, &VAO_chartBest);
    glDeleteBuffers(1, &VBO_chartBest);
    glDeleteVertexArrays(1, &VAO_chartAvg);
    glDeleteBuffers(1, &VBO_chartAvg);
	glDeleteVertexArrays(1, &VAO_axes);
    glDeleteBuffers(1, &VBO_axes);
    glDeleteProgram(prog_edges);
    glDeleteProgram(prog_nodes);
    glDeleteProgram(prog_route);
    glDeleteProgram(prog_chartBest);
    glDeleteProgram(prog_chartAvg);
	glDeleteProgram(prog_axes);
	txt.cleanup();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}