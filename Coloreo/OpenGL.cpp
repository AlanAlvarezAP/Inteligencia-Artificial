#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

int cant_colores = 3;

struct Nodo {
    float x, y;
    int id;
    int color = 0;
    std::vector<int> vecinos;
};

std::vector<Nodo> grafo;
int numNodos;
bool huboBacktrack = false;
int contadorBacktracks = 0;

// Colores: 0:Gris, 1:Rojo, 2:Verde, 3:Azul, 4:Amarillo, 5:Magenta
float paleta[6][3] = {
    {0.4f, 0.4f, 0.4f}, 
    {1.0f, 0.0f, 0.0f}, 
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.6f, 1.0f}, 
    {1.0f, 1.0f, 0.0f}, 
    {1.0f, 0.0f, 1.0f}
};

bool esSeguro(int nodoIdx, int col) {
    for (int vIdx : grafo[nodoIdx].vecinos) {
        if (grafo[vIdx].color == col) return false;
    }
    return true;
}

void limpiarColores() {
    for (auto& n : grafo) n.color = 0;
    huboBacktrack = false;
    contadorBacktracks = 0;
}

int obtenerNodoMRV() {
    int mejorNodo = -1;
    int minOpciones = 999;

    for (int i = 0; i < numNodos; i++) {
        if (grafo[i].color == 0) {
            int opcionesLegales = 0;
            for (int c = 1; c <= 5; c++) {
                if (esSeguro(i, c)) opcionesLegales++;
            }
            if (opcionesLegales < minOpciones) {
                minOpciones = opcionesLegales;
                mejorNodo = i;
            }
        }
    }
    return mejorNodo;
}

// solver variable mas restrictiva
bool solverOrdenFijo(std::vector<int>& orden, int pos) {
    if (pos == orden.size()) return true;

    int nodoIdx = orden[pos];
    for (int c = 1; c <= cant_colores; c++) {
        if (esSeguro(nodoIdx, c)) {
            grafo[nodoIdx].color = c;
            if (solverOrdenFijo(orden, pos + 1)) return true;

            // backtrack
            huboBacktrack = true;
            contadorBacktracks++;
            grafo[nodoIdx].color = 0;
        }
    }
    return false;
}

// solver variable mas restringida
bool solverMRV() {
    int nodoIdx = obtenerNodoMRV();
    if (nodoIdx == -1) return true;

    for (int c = 1; c <= cant_colores; c++) {
        if (esSeguro(nodoIdx, c)) {
            grafo[nodoIdx].color = c;
            if (solverMRV()) return true;

            huboBacktrack = true;
            contadorBacktracks++;
            grafo[nodoIdx].color = 0;
        }
    }
    return false;
}

void generarGrafo() {
    srand(time(0));
    grafo.clear();
    std::cout << "\n--- GENERANDO GRAFO ---" << std::endl;

    for (int i = 0; i < numNodos; i++) {
        grafo.push_back({ (float)(rand() % 180 + 10), (float)(rand() % 180 + 10), i, 0 });
    }

    for (int i = 0; i < numNodos; i++) {
        int objetivo = (rand() % 4) + 2;
        int intentos = 0;
        while (grafo[i].vecinos.size() < objetivo && intentos < 50) {
            int c = rand() % numNodos;
            if (c != i && grafo[c].vecinos.size() < 5) {
                bool yaExiste = false;
                for (int v : grafo[i].vecinos) if (v == c) yaExiste = true;
                if (!yaExiste) {
                    grafo[i].vecinos.push_back(c);
                    grafo[c].vecinos.push_back(i);
                }
            }
            intentos++;
        }
        std::cout << "Nodo " << i << " -> " << grafo[i].vecinos.size() << " relaciones. (Pos: " << grafo[i].x << "," << grafo[i].y << ")" << std::endl;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Z) {
            std::cout << "\n[Z] Ejecutando: Variable Mas Restrictiva (Por Grado)..." << std::endl;
            limpiarColores();
            std::vector<int> orden;
            for (int i = 0; i < numNodos; i++) orden.push_back(i);
            std::sort(orden.begin(), orden.end(), [](int a, int b) {
                return grafo[a].vecinos.size() > grafo[b].vecinos.size();
                });
            solverOrdenFijo(orden, 0);
            std::cout << "Backtracking usado: " << (huboBacktrack ? "SI" : "NO") << " (Total fallos: " << contadorBacktracks << ")" << std::endl;
        }
        if (key == GLFW_KEY_X) {
            std::cout << "\n[X] Ejecutando: Variable Mas Restringida (MRV)..." << std::endl;
            limpiarColores();
            solverMRV();
            std::cout << "Backtracking usado: " << (huboBacktrack ? "SI" : "NO") << " (Total fallos: " << contadorBacktracks << ")" << std::endl;
        }
    }
}

int main() {
    std::cout << "Ingrese cantidad de nodos: "; std::cin >> numNodos;
    if (!glfwInit()) return -1;

    generarGrafo();

    GLFWwindow* window = glfwCreateWindow(800, 800, "Z: Restrictiva | X: Restringida", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glOrtho(-5, 205, -5, 205, -1, 1);

        // Dibujar Aristas
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINES);
        for (const auto& n : grafo) {
            for (int vIdx : n.vecinos) {
                glVertex2f(n.x, n.y); glVertex2f(grafo[vIdx].x, grafo[vIdx].y);
            }
        }
        glEnd();

        // Dibujar Nodos
        glPointSize(22.0f);
        glBegin(GL_POINTS);
        for (const auto& n : grafo) {
            glColor3f(paleta[n.color][0], paleta[n.color][1], paleta[n.color][2]);
            glVertex2f(n.x, n.y);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}