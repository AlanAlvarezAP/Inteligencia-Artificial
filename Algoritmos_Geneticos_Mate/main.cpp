#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Genetico.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

const int WINDOW_WIDTH=800,WINDOW_HEIGHT=600;

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
    glViewport(0,0,width,height);
}
void key_callback(GLFWwindow* window,int key,int scan,int action,int mods){
    switch(key){
        case GLFW_KEY_C:{
            if(mods & GLFW_MOD_CONTROL){
                std::cout << "CTRL+C pressed leaving ... " << std::endl;
                glfwSetWindowShouldClose(window,GLFW_TRUE);
            }
            break;
        }
        case GLFW_KEY_ESCAPE:{
            std::cout << "ESC pressed leaving... " << std::endl;
            glfwSetWindowShouldClose(window,GLFW_TRUE);
            break;
        }
        default: break;
    }
}

const char* vert_src =
	"#version 330 core\n"
	"layout(location=0) in vec2 pos;\n"
	"uniform vec4 u_color;\n"
	"void main(){ gl_Position = vec4(pos, 0.0, 1.0); }\n";

const char* frag_src =
	"#version 330 core\n"
	"uniform vec4 u_color;\n"
	"out vec4 FragColor;\n"
	"void main(){ FragColor = u_color; }\n";

GLuint make_shader(){
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs,1,&vert_src,NULL); glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs,1,&frag_src,NULL); glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog,vs); glAttachShader(prog,fs);
    glLinkProgram(prog);
    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

std::vector<float> build_line(
    const std::vector<std::pair<double,double>>& data,
    bool use_best,
    double min_y,
    double max_y)
{
    int n = data.size();
    std::vector<float> verts;
    verts.reserve(n * 2);

    double range = max_y - min_y;
    if (range < 1e-12) range = 1.0;

    for(int i = 0; i < n; i++){
        float x = -0.9f + 1.8f * i / (float)((n > 1) ? (n - 1) : 1);

        double v = use_best ? data[i].first: data[i].second;
        float y = -0.9f + 1.8f * (float)((v - min_y) / range);

        verts.push_back(x);
        verts.push_back(y);
    }
    return verts;
}

void draw_label(GLuint prog, GLint u_color, const char* text, float x, float y) {
    static char buf[5000];
    int vertex = stb_easy_font_print(0, 0, (char*)text, NULL, buf, sizeof(buf));

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao); 
	glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex * 4 * 4 * sizeof(float), buf, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float sx = 1.0f / 400.0f, sy = 1.0f / 300.0f;
    glUseProgram(prog);
    glUniform4f(u_color, 1.f, 1.f, 1.f, 1.f);

    float* verts = (float*)buf;
    std::vector<float> pts;
    for (int i = 0; i < vertex * 4; i++) {
        pts.push_back(x + verts[i*4+0] * sx);
        pts.push_back(y - verts[i*4+1] * sy);
    }
	
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(float), pts.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < vertex; i++)
        glDrawArrays(GL_LINES, i * 4, 4);

    glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo);
}

void draw_graph(const std::vector<std::pair<double,double>>& data){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,"Fitness por Generacion", NULL, NULL);
    if(!window){ glfwTerminate(); return; }
    glfwMakeContextCurrent(window);

    if(!gladLoadGL(glfwGetProcAddress)){
        std::cout << "Failed to load GLAD" << std::endl;
        glfwTerminate(); return;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    GLuint prog = make_shader();
    GLint u_color = glGetUniformLocation(prog, "u_color");

	double min_y =  1e18, max_y = -1e18;

	for(auto& p : data){
		min_y = std::min({min_y, p.first, p.second});
		max_y = std::max({max_y, p.first, p.second});
	}

	if(max_y - min_y < 1e-12) max_y = min_y + 1.0;

	auto best_verts = build_line(data, true,  min_y, max_y);
	auto avg_verts  = build_line(data, false, min_y, max_y);

    GLuint vao_best, vbo_best;
    glGenVertexArrays(1,&vao_best); glGenBuffers(1,&vbo_best);
    glBindVertexArray(vao_best);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_best);
    glBufferData(GL_ARRAY_BUFFER,
    best_verts.size()*sizeof(float), best_verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    GLuint vao_avg, vbo_avg;
    glGenVertexArrays(1,&vao_avg); glGenBuffers(1,&vbo_avg);
    glBindVertexArray(vao_avg);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_avg);
    glBufferData(GL_ARRAY_BUFFER,
    avg_verts.size()*sizeof(float), avg_verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    float axes[] = {
		-0.9f,  0.0f,   
		0.9f,  0.0f,
        0.0f, -0.9f,
		0.0f,  0.9f
    };
	
    float axes2[] = {
        -0.9f, -0.9f,   
		0.9f, -0.9f,   
        -0.9f, -0.9f,
		-0.9f,  0.9f    
    };
    GLuint vao_ax, vbo_ax;
    glGenVertexArrays(1,&vao_ax); 
	glGenBuffers(1,&vbo_ax);
    glBindVertexArray(vao_ax);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ax);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes2), axes2, GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);


    while(!glfwWindowShouldClose(window)){
		
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);

        glUniform4f(u_color, 1.f,1.f,1.f,1.f);
        glBindVertexArray(vao_ax);
        glDrawArrays(GL_LINES, 0, 4);

        glUniform4f(u_color, 1.f,0.9f,0.f,1.f);
        glBindVertexArray(vao_best);
        glDrawArrays(GL_LINE_STRIP, 0, best_verts.size()/2);

        glUniform4f(u_color, 0.f,0.9f,1.f,1.f);
        glBindVertexArray(vao_avg);
        glDrawArrays(GL_LINE_STRIP, 0, avg_verts.size()/2);

		draw_label(prog, u_color, "generacion", 0.3f, -0.93f);
		draw_label(prog, u_color, "fitness", -0.88f, 0.95f);
				

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1,&vao_best); 
	glDeleteBuffers(1,&vbo_best);
    glDeleteVertexArrays(1,&vao_avg);
	glDeleteBuffers(1,&vbo_avg);
    glDeleteVertexArrays(1,&vao_ax);
	glDeleteBuffers(1,&vbo_ax);
    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void print_menu() {
    std::cout << "===================================" << std::endl;
    std::cout << "|        Bienvenido a             |" << std::endl;
    std::cout << "| Algoritmos geneticos maximizador|" << std::endl;
    std::cout << "|                                 |" << std::endl;
    std::cout << "|  1. Simular Algoritmo           |" << std::endl;
    std::cout << "|  2. Generar graficos            |" << std::endl;
    std::cout << "|  3. Salir                       |" << std::endl;
    std::cout << "===================================" << std::endl;
}

int main(int argc,char* argv[]){
    Genetico Alg_Gen;
    bool exec = false;

    while(true){
        print_menu();
        int option; std::cin >> option;

        if(option == 1){
            Alg_Gen.Run_Genetics(50);
            exec = true;
			for(auto& pair: Alg_Gen.best_and_avg){
				std::cout << pair.first << " ";
			}
			std::cout << std::endl;
			
        } else if(option == 2){
            if(!exec){
                std::cout << "Primero ejecute el algoritmo" << std::endl;
                continue;
            }
            draw_graph(Alg_Gen.best_and_avg);
        } else {
            break;
        }
    }
    return 0;
}