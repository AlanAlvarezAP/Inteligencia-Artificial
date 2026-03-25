#include "utils.h"
#include <glad/gl.h>
#include "Graph.h"

extern int WINDOW_WIDTH, WINDOW_HEIGHT;
extern float zoom;
extern GLuint program_id;
extern GLuint VAO;
extern std::vector<Vertex> vertices;

void dibujarPantalla(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(program_id);
    
    GLint screenCordLoc = glGetUniformLocation(program_id, "screenCords");
    if (screenCordLoc != -1) {
        glUniform2f(screenCordLoc, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
    }
    
    GLint escalaLoc = glGetUniformLocation(program_id, "escala");
    if (escalaLoc != -1) {
        glUniform1f(escalaLoc, zoom);
    }
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertices.size());
    
    glfwSwapBuffers(window);
}

void esperarConEventos(GLFWwindow* window, int milisegundos) {
    auto inicio = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - inicio < std::chrono::milliseconds(milisegundos)) {
        glfwPollEvents();
        if (glfwWindowShouldClose(window)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

Node::Node() :index(-1), x(FLOAT_MAX), y(FLOAT_MAX), dist_dest(FLOAT_MAX) {};
Node::Node(int idx) :index(idx), x(FLOAT_MAX), y(FLOAT_MAX), dist_dest(FLOAT_MAX) {};
Node::Node(int idx,float xx,float yy) :index(idx), x(xx), y(yy), dist_dest(FLOAT_MAX) {};


float dist_calculation(std::pair<float,float> cord1,std::pair<float,float> cord2) {
    return std::sqrt((cord2.first - cord1.first) * (cord2.first - cord1.first) + (cord2.second - cord1.second) * (cord2.second - cord1.second));
}


void Graph::Fill_Graph(std::vector<Vertex> &vex,std::map<std::pair<int,int>,int> &offset_amount,std::map<std::pair<float,float>,std::vector<int>> &offset_by_node) {
    for (auto node : graph) {
        delete node;
    }
    graph.clear();
	get_node.clear();
	
	for (int i = 0;i < 21;i++) {
        for (int j = 0;j < 21;j++) {
            int index = i * 21 + j;
            Node* tmp = new Node(index, i*10, j*10);
            graph.push_back(tmp);
			get_node[{i*10,j*10}]=index;
        }
    }
	int off=0;
	int off_2=0;
	float gris=0.5f;
    for (int i = 0;i < 21;i++) {
        for (int j = 0;j < 21;j++) {
            int index = i * 21 + j;
            Node* actu = graph[index];
            if(j + 1 < 21) {
                int tmp_idx = i * 21 + (j+1);
                Node* tmp_node = graph[tmp_idx];
                actu->nigh.push_back(tmp_node);
                tmp_node->nigh.push_back(actu);
				vex.push_back({(float)i*10,(float)j*10,0.0f,gris,gris,0.0f});
				vex.push_back({(float)i*10,(float)(j+1)*10,0.0f,gris,gris,0.0f});
				offset_by_node[{(float)i*10,(float)j*10}].push_back(off_2++);
				offset_by_node[{(float)i*10,(float)(j+1)*10}].push_back(off_2++);
				offset_amount[{index,tmp_idx}]=off++;
            }
            if (i + 1 < 21 && j + 1 < 21) {
                int tmp_idx = (i+1) * 21 + (j+1);
                Node* tmp_node = graph[tmp_idx];
                actu->nigh.push_back(tmp_node);
                tmp_node->nigh.push_back(actu);
				vex.push_back({(float)i*10,(float)j*10,0.0f,gris,gris,0.0f});
				vex.push_back({(float)(i+1)*10,(float)(j+1)*10,0.0f,gris,gris,0.0f});
				offset_by_node[{(float)i*10,(float)j*10}].push_back(off_2++);
				offset_by_node[{(float)(i+1)*10,(float)(j+1)*10}].push_back(off_2++);
				offset_amount[{index,tmp_idx}]=off++;
            }
            if (i + 1 < 21) {
                int tmp_idx = (i+1) * 21 + j;
                Node* tmp_node = graph[tmp_idx];
                actu->nigh.push_back(tmp_node);
                tmp_node->nigh.push_back(actu);
				vex.push_back({(float)i*10,(float)j*10,0.0f,gris,gris,0.0f});
				vex.push_back({(float)(i+1)*10,(float)j*10,0.0f,gris,gris,0.0f});
				offset_by_node[{(float)i*10,(float)j*10}].push_back(off_2++);
				offset_by_node[{(float)(i+1)*10,(float)j*10}].push_back(off_2++);
				offset_amount[{index,tmp_idx}]=off++;
            }
            if (i + 1 < 21 && j - 1 >= 0) {
                int tmp_idx = (i+1) * 21 + (j-1);
                Node* tmp_node = graph[tmp_idx];
                actu->nigh.push_back(tmp_node);
                tmp_node->nigh.push_back(actu);
				vex.push_back({(float)i*10,(float)j*10,0.0f,gris,gris,0.0f});
				vex.push_back({(float)(i+1)*10,(float)(j-1)*10,0.0f,gris,gris,0.0f});
				offset_by_node[{(float)i*10,(float)j*10}].push_back(off_2++);
				offset_by_node[{(float)(i+1)*10,(float)(j-1)*10}].push_back(off_2++);
				offset_amount[{index,tmp_idx}]=off++;
            }
        }
    }
}

void Graph::print_grade() {
    for (int i = 0;i < 21;i++) {
        for (int j = 0;j < 21;j++) {
            int index = i * 21 + j;
            std::cout << graph[index]->nigh.size() << " ";
        }
        std::cout << std::endl;
    }
}

void Graph::Fill_Heuristics(int end_node) {
    for (int i = 0;i < 21;i++) {
        for (int j = 0;j < 21;j++) {
            int index = i * 21 + j;
            Node* node = graph[index];
            node->dist_dest = dist_calculation({node->x,node->y}, {graph[end_node]->x,graph[end_node]->y});
        }
    }
}

void Graph::BFS(int start_index,int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window) {
    auto start_time = std::chrono::high_resolution_clock::now();
	
	std::queue<Traversal> que;
    std::vector<bool> visited(graph.size(),false);
    que.push(Traversal{ graph[start_index],{graph[start_index]},0});
    visited[start_index] = true;
	
	int vertices_revisados=0;
	
    while (!que.empty()) {
        auto top = que.front();
        vertices_revisados++;
		que.pop();
		
		updateColorNode({graph[top.origin->index]->x,graph[top.origin->index]->y},0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
		dibujarPantalla(window);
		esperarConEventos(window, 10);
		
        if (top.origin->index == end_index) {
            for (auto p : top.path_to_origin) {
                updateColorNode({p->x,p->y},1.0f,0.0f,0.0f,vertices,offset_by_node,VBO);
				dibujarPantalla(window);
				esperarConEventos(window, 10);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << std::endl;
			std::cout << "================================= " << std::endl;
			std::cout << " RESULTADOS BFS " << std::endl;
            std::cout << " El costo de todo el viaje fue de: " << top.actu_trav << std::endl;
			std::cout << " Los nodos visitados fueron: " << vertices_revisados << std::endl;
			std::cout << " Tiempo que se tardo: " << duration.count() << "s " << std::endl;
			std::cout << "================================= " << std::endl;
            return;
        }
        
        for (auto p : top.origin->nigh) {
            if (!visited[p->index]) {
                visited[p->index] = true;
                auto new_path = top.path_to_origin;
                new_path.push_back(p);
                float new_cost = top.actu_trav + dist_calculation({ p->x,p->y }, { top.origin->x,top.origin->y });
                que.push(Traversal{ p,new_path,new_cost});
			}
        }
    }
    std::cout << "No encontrado" << std::endl;
}

void Graph::DFS(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window) {
    auto start_time = std::chrono::high_resolution_clock::now();
	
	std::stack<Traversal> stac;
    std::vector<bool> visited(graph.size(), false);
    stac.push(Traversal{ graph[start_index],{graph[start_index]},0});
    visited[start_index] = true;
	
	int vertices_revisados=0;
	
	
    while (!stac.empty()) {
        auto top = stac.top();
		vertices_revisados++;
        stac.pop();
		
		updateColorNode({graph[top.origin->index]->x,graph[top.origin->index]->y},0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
		dibujarPantalla(window);
		esperarConEventos(window, 10);
		
        if (top.origin->index == end_index) {
            for (auto p : top.path_to_origin) {
                updateColorNode({p->x,p->y},1.0f,0.0f,0.0f,vertices,offset_by_node,VBO);
				dibujarPantalla(window);
				esperarConEventos(window, 10);
            }
			auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << std::endl;
			std::cout << "================================= " << std::endl;
			std::cout << " RESULTADOS DFS " << std::endl;
            std::cout << " El costo de todo el viaje fue de: " << top.actu_trav << std::endl;
			std::cout << " Los nodos visitados fueron: " << vertices_revisados << std::endl;
			std::cout << " Tiempo que se tardo: " << duration.count() << "s " << std::endl;
			std::cout << "================================= " << std::endl;
            return;
        }

        for (auto p : top.origin->nigh) {
            if (!visited[p->index]) {
                visited[p->index] = true;
                auto new_path = top.path_to_origin;
                new_path.push_back(p);
                float new_cost = top.actu_trav + dist_calculation({ p->x,p->y }, { top.origin->x,top.origin->y });
                stac.push({ p,new_path,new_cost });
				
            }
        }
    }
    std::cout << "No encontrado" << std::endl;

}


void Graph::Hillclimbing(int start_index,int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window) {
    auto start_time = std::chrono::high_resolution_clock::now();
	std::priority_queue<Traversal,std::vector<Traversal>,Traversal::Hill_Climb_Comp> path;
    std::vector<bool> visited(graph.size(), false);
    path.push({ graph[start_index],{graph[start_index]},0 });

    // Llenar la heuristicas
    Fill_Heuristics(end_index);
	int vertices_revisados=0;

    while (!path.empty()) {
        auto top = path.top();
		vertices_revisados++;
        path.pop();

        if (visited[top.origin->index]) {
            continue;
        }

        visited[top.origin->index] = true;

		updateColorNode({graph[top.origin->index]->x,graph[top.origin->index]->y},0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
		dibujarPantalla(window);
		esperarConEventos(window, 50);

        if (top.origin->index == end_index) {
            for (auto p : top.path_to_origin) {
                updateColorNode({p->x,p->y},1.0f,0.0f,0.0f,vertices,offset_by_node,VBO);
				dibujarPantalla(window);
				esperarConEventos(window, 50);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << std::endl;
			std::cout << "================================= " << std::endl;
			std::cout << " RESULTADOS Hill Climbing " << std::endl;
            std::cout << " El costo de todo el viaje fue de: " << top.actu_trav << std::endl;
			std::cout << " Los nodos visitados fueron: " << vertices_revisados << std::endl;
			std::cout << " Tiempo que se tardo: " << duration.count() << "s " << std::endl;
			std::cout << "================================= " << std::endl;
            return;
        }

        for (auto p : top.origin->nigh) {
            if (!visited[p->index]) {
                auto new_path = top.path_to_origin;
                new_path.push_back(p);
                float new_cost = top.actu_trav + dist_calculation({ p->x,p->y }, { top.origin->x,top.origin->y });
                path.push({ p,new_path,new_cost });
				
            }
            
        }
    }
    std::cout << "No encontrado" << std::endl;
}

void Graph::A_star(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window) {
    auto start_time = std::chrono::high_resolution_clock::now();
	
	std::priority_queue<Traversal, std::vector<Traversal>, Traversal::A_star_Comp> path;
    std::vector<bool> visited(graph.size(), false);
    path.push({ graph[start_index],{graph[start_index]},0 });

    // Llenar la heuristicas
    Fill_Heuristics(end_index);

	int vertices_revisados=0;

    while (!path.empty()) {
        auto top = path.top();
		vertices_revisados++;
        path.pop();

        if (visited[top.origin->index]) {
            continue;
        }

        visited[top.origin->index] = true;

		updateColorNode({graph[top.origin->index]->x,graph[top.origin->index]->y},0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
		dibujarPantalla(window);
		esperarConEventos(window, 50);

        if (top.origin->index == end_index) {
            for (auto p : top.path_to_origin) {
                updateColorNode({p->x,p->y,},1.0f,0.0f,0.0f,vertices,offset_by_node,VBO);
				dibujarPantalla(window);
				esperarConEventos(window, 50);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << std::endl;
			std::cout << "================================= " << std::endl;
			std::cout << " RESULTADOS A* " << std::endl;
            std::cout << " El costo de todo el viaje fue de: " << top.actu_trav << std::endl;
			std::cout << " Los nodos visitados fueron: " << vertices_revisados << std::endl;
			std::cout << " Tiempo que se tardo: " << duration.count() << "s " << std::endl;
			std::cout << "================================= " << std::endl;
            return;
        }

        for (auto p : top.origin->nigh) {
            if (!visited[p->index]) {
                auto new_path = top.path_to_origin;
                new_path.push_back(p);
                float new_cost = top.actu_trav + dist_calculation({ p->x,p->y }, { top.origin->x,top.origin->y });
                path.push({ p,new_path,new_cost });
				
            }

        }
    }
    std::cout << "No encontrado" << std::endl;
}


// Eliminate 20% so is 88 nodes out of 440 nodes
void Graph::Eliminate_random(std::vector<Vertex>& vertices, std::map<std::pair<int,int>, int>& offset_amount,std::map<std::pair<float,float>,std::vector<int>> &offset_by_node) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(0, 440);

    int conta = 0;

    while (conta < 88) {
        int selected_idx = -1;
        do {
            selected_idx = dist(gen);
        } while (graph[selected_idx]->nigh.size() == 0);
        //std::cout << "Se eliminara el nodo: " << selected_idx << std::endl;
        for (auto p : graph[selected_idx]->nigh) {
            Node* nghb = graph[p->index];
            nghb->nigh.erase(std::remove(nghb->nigh.begin(), nghb->nigh.end(), graph[selected_idx]), nghb->nigh.end());
        }
        graph[selected_idx]->nigh.clear();
        conta++;
    }

	vertices.clear();
    offset_amount.clear();
    offset_by_node.clear();
	float gris=0.5f;
	int vertexidx=0;
    for (int i = 0; i < graph.size(); i++) {
		if (graph[i]->nigh.empty()) {
            continue;
        }
        for (auto neighbor : graph[i]->nigh) {
            if (i < neighbor->index) {
                offset_amount[{i, neighbor->index}] = vertexidx;
                
                vertices.push_back({graph[i]->x, graph[i]->y, 0.0f, gris, gris, 0.0f});
                offset_by_node[{graph[i]->x, graph[i]->y}].push_back(vertexidx);
                vertexidx++;
                
                vertices.push_back({neighbor->x, neighbor->y, 0.0f, gris, gris, 0.0f});
                offset_by_node[{neighbor->x, neighbor->y}].push_back(vertexidx);
                vertexidx++;
            }
        }
    }

}