/*
	Pasos basicos para OpenGL
	1. Inicializar GLFW y el perfil a usar
	2. Configurar Ventana para hacer contexto esa ventana
	3. Cargar GLAD
	4. Colocar Callbacks
	5. Bucle principal de rendering 
	5.1 Swapear buffers
	5.2 Buscar eventos
*/

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Graph.h"
#include "utils.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>
#include <string>


typedef std::pair<int,int> Pares;

int WINDOW_WIDTH=800,WINDOW_HEIGHT=600;
float zoom=std::min((float)WINDOW_WIDTH/210.0f,(float)WINDOW_HEIGHT/210.0f)*0.7f;
GLuint vertex_shader_id,fragment_shader_id,program_id;
GLuint VAO,VBO;
Graph grafito;
std::vector<Vertex> vertices,old_vertices;
std::map<Pares,int> offset_amount,old_offset;
std::map<std::pair<float,float>,std::vector<int>> offset_by_node,old_offset_by_node;
bool starting=true;
std::pair<float,float> start_node={-1,-1},end_node={-1,-1};


const char* vertex_shader="#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aCol;\n"
"uniform vec2 screenCords;\n"
"uniform float escala;\n"
"out vec3 ouColor;\n"
"void main(){\n"
"	float x=((aPos.x-105.0f)*escala+(screenCords.x/2));\n"
"	float y=((aPos.y-105.0f)*escala+(screenCords.y/2));\n"
"	float NDC_x=(x/screenCords.x)*2.0f-1.0f;\n"
"	float NDC_y=(y/screenCords.y)*2.0f-1.0f;\n"
"	gl_Position=vec4(NDC_x,NDC_y,aPos.z,1.0);\n"
"	ouColor=aCol;\n"	
"}\n";

const char* fragment_shader="#version 330 core\n"
"in vec3 ouColor;\n"
"out vec4 fragCol;\n"
"void main(){\n"
"	fragCol=vec4(ouColor.rgb,1.0f);\n"
"}\n";

float mouse_change_NDCX(double mouseX){
	return (mouseX/WINDOW_WIDTH)*2.0f-1.0f;
}

float mouse_change_NDCY(double mouseY){
	return 1.0f-(mouseY/WINDOW_HEIGHT)*2.0f;
}

float NDC_screen_X(float ndcX){
	return WINDOW_WIDTH * (ndcX+1.0f)/2.0f;
}

float NDC_screen_Y(float ndcY){
	return WINDOW_HEIGHT * (ndcY+1.0f)/2.0f;
}

float screen_normalX(float screenx){
	return (screenx-WINDOW_WIDTH/2.0f)/zoom+105.0f;
}

float screen_normalY(float screenY){
	return (screenY-WINDOW_HEIGHT/2.0f)/zoom+105.0f;
}

std::pair<float,float> ClosestNode(double mouseX,double mouseY){
	float final_x=screen_normalX(NDC_screen_X(mouse_change_NDCX(mouseX)));
	float final_y=screen_normalY(NDC_screen_Y(mouse_change_NDCY(mouseY)));
	
	std::pair<float,float> close_node={-1,-1};
	int max_rad=5.0f;
	
	for (int i = 0;i < 21;i++) {
        for (int j = 0;j < 21;j++) {
            int index=i*21+j;
			
			if((int)grafito.graph[index]->nigh.size() > 0){
				float cord_x=grafito.graph[index]->x;
				float cord_y=grafito.graph[index]->y;
				float dist=std::sqrt((final_x-cord_x)*(final_x-cord_x)+(final_y-cord_y)*(final_y-cord_y));
				
				if(dist < max_rad){
					close_node={cord_x,cord_y};
					max_rad=dist;
				}
			}
			
			
        }
    }
	return close_node;
}



void framebuffer_size_callback(GLFWwindow* window,int width,int height){
	if (width <= 0 || height <= 0) {
        return;
    }
	glViewport(0,0,width,height);
	
	WINDOW_HEIGHT=height;
	WINDOW_WIDTH=width;
	
	zoom=std::min((float)WINDOW_WIDTH/210.0f,(float)WINDOW_HEIGHT/210.0f)*0.9f;
}



void mouse_button_callback(GLFWwindow* window,int button,int action,int mods){
	if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS){
		if (grafito.graph.empty()) {
            std::cout << "Grafo vacio genera uno primero (tecla 1)" << std::endl;
            return;
        }
		
		double mouse_x,mouse_y;
		glfwGetCursorPos(window,&mouse_x,&mouse_y);
		
		std::pair<float,float> Node=ClosestNode(mouse_x,mouse_y);
		if(Node.first != -1 && Node.second != -1){
			
			if(starting){
				if((start_node.first!=-1 && start_node.second !=-1) && (start_node != Node)){
					updateColorNode(start_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				}
				start_node=Node;
				updateColorNode(start_node,0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
				starting=false;
			}else{
				if((end_node.first!=-1 && end_node.second != -1) && (end_node != Node)){
					updateColorNode(end_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				}
				end_node=Node;
				updateColorNode(end_node,0.0f,0.0f,1.0f,vertices,offset_by_node,VBO);
				starting=true;
			}
			
		}
	}
}

void updateVertices(std::vector<Vertex>& vertices, GLuint VBO) {
	if (vertices.empty()) {
        std::cout << "updateVertices: vertices vacio, ignorando" << std::endl;
        return;
    }
    if (VBO == 0) {
        std::cout << "updateVertices: VBO invalido (0)" << std::endl;
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
}

bool ensure_Simulation(){
	if(start_node == end_node){
		return false;
	}
	if((start_node == std::pair<float,float>{-1.0f,-1.0f}) || (end_node == std::pair<float,float>{-1.0f,-1.0f})){
		return false;
	}
	return true;
}

void setup_shaders(GLuint &vertex_shader_id,GLuint &fragment_shader_id, GLuint &program_id){
	int sucess;
	char infoLog[512];
	
	vertex_shader_id=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_id,1,&vertex_shader,NULL);
	glCompileShader(vertex_shader_id);
	
	glGetShaderiv(vertex_shader_id,GL_COMPILE_STATUS,&sucess);
	if(!sucess){
		glGetShaderInfoLog(vertex_shader_id,512,NULL,infoLog);
		std::cout << " ERROR al compilar vertex shader :/ -> " << infoLog << std::endl;
	}
	
	fragment_shader_id=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_id,1,&fragment_shader,NULL);
	glCompileShader(fragment_shader_id);
	
	glGetShaderiv(fragment_shader_id,GL_COMPILE_STATUS,&sucess);
	if(!sucess){
		glGetShaderInfoLog(fragment_shader_id,512,NULL,infoLog);
		std::cout << " ERROR al compilar el fragment shader :/ -> " << infoLog << std::endl;
	}
	
	program_id=glCreateProgram();
	glAttachShader(program_id,vertex_shader_id);
	glAttachShader(program_id,fragment_shader_id);
	glLinkProgram(program_id);
	
	glGetProgramiv(program_id,GL_LINK_STATUS,&sucess);
	if(!sucess){
		glGetProgramInfoLog(program_id,512,NULL,infoLog);
		std::cout << " ERROR al linkear los shaders :/ -> " << infoLog << std::endl;
	}
	
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
}

void Set_Vs(std::vector<Vertex> vertices,GLuint &VAO,GLuint &VBO){
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
}

void key_callback(GLFWwindow* window,int key,int scan,int action,int mods){
	if(action == GLFW_PRESS){
		switch(key){
			case GLFW_KEY_1:{
				if (VBO == 0) {
                    std::cout << "VBO no inicializado. Espera a que termine la carga inicial." << std::endl;
                    break;
                }
				std::cout << "Generando grafo... " << std::endl;
				if(!vertices.empty() || !old_offset.empty()){
					vertices.clear();
					offset_amount.clear();
					offset_by_node.clear();
					old_vertices.clear();
					old_offset.clear();
					old_offset_by_node.clear();
				}
				start_node=end_node={-1,-1};
				grafito.Fill_Graph(vertices,offset_amount,offset_by_node);
				updateVertices(vertices, VBO);
				old_vertices=vertices;
				old_offset=offset_amount;
				old_offset_by_node=offset_by_node;
				updateColorNode(start_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				updateColorNode(end_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				break;
			}
			case GLFW_KEY_2:{
				std::cout << "Eliminando 20% de nodos..." << std::endl;
				grafito.Eliminate_random(vertices,offset_amount,offset_by_node);
				updateVertices(vertices, VBO);
				old_vertices=vertices;
				old_offset=offset_amount;
				old_offset_by_node=offset_by_node;
				start_node=end_node={-1,-1};
				updateColorNode(start_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				updateColorNode(end_node,0.5f,0.5f,0.0f,vertices,offset_by_node,VBO);
				break;
			}
			case GLFW_KEY_3:{
				if(ensure_Simulation()){
					std::cout << "Ejecutando BFS..." << std::endl;
					if(!old_offset.empty()){
						vertices=old_vertices;
						offset_amount=old_offset;
						offset_by_node=old_offset_by_node;
						updateColorNode(start_node,0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
						updateColorNode(end_node,0.0f,0.0f,1.0f,vertices,offset_by_node,VBO);
						updateVertices(vertices, VBO);
					}
					grafito.BFS(grafito.get_node[start_node],grafito.get_node[end_node],vertices,offset_by_node,(unsigned int)VBO,window);
				}else{
					std::cout << "Asegurate tener los puntos de inicio y fin validos " << std::endl;
				}
				
				break;
			}
			case GLFW_KEY_4:{
				if(ensure_Simulation()){
					std::cout << "Ejecutando DFS..." << std::endl;
					if(!old_offset.empty()){
						vertices=old_vertices;
						offset_amount=old_offset;
						offset_by_node=old_offset_by_node;
						updateColorNode(start_node,0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
						updateColorNode(end_node,0.0f,0.0f,1.0f,vertices,offset_by_node,VBO);
						updateVertices(vertices, VBO);
					}
					grafito.DFS(grafito.get_node[start_node],grafito.get_node[end_node],vertices,offset_by_node,(unsigned int)VBO,window);
				}else{
					std::cout << "Asegurate tener los puntos de inicio y fin validos " << std::endl;
				}
				
				break;
			}
			case GLFW_KEY_5:{
				if(ensure_Simulation()){
					std::cout << "Ejecutando Hill Climbing..." << std::endl;
					if(!old_offset.empty()){
						vertices=old_vertices;
						offset_amount=old_offset;
						offset_by_node=old_offset_by_node;
						updateColorNode(start_node,0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
						updateColorNode(end_node,0.0f,0.0f,1.0f,vertices,offset_by_node,VBO);
						updateVertices(vertices, VBO);
					}
					grafito.Hillclimbing(grafito.get_node[start_node],grafito.get_node[end_node],vertices,offset_by_node,(unsigned int)VBO,window);
				}else{
					std::cout << "Asegurate tener los puntos de inicio y fin validos " << std::endl;
				}
				
				break;
			}
			case GLFW_KEY_6:{
				if(ensure_Simulation()){
					std::cout << "Ejecutando A*..." << std::endl;
					if(!old_offset.empty()){
						vertices=old_vertices;
						offset_amount=old_offset;
						offset_by_node=old_offset_by_node;
						updateColorNode(start_node,0.0f,1.0f,0.0f,vertices,offset_by_node,VBO);
						updateColorNode(end_node,0.0f,0.0f,1.0f,vertices,offset_by_node,VBO);
						updateVertices(vertices, VBO);
					}
					grafito.A_star(grafito.get_node[start_node],grafito.get_node[end_node],vertices,offset_by_node,(unsigned int)VBO,window);
				}else{
					std::cout << "Asegurate tener los puntos de inicio y fin validos " << std::endl;
				}
				
				break;
			}
			case GLFW_KEY_7:{
				std::cout << "Saliendo inmediatamente..." << std::endl;
				glfwSetWindowShouldClose(window,GLFW_TRUE);
				break;
			}
			default:
				break;
		}
	}
}

void print_menu() {
    std::cout << "===================================" << std::endl;
    std::cout << "|        Bienvenido a             |" << std::endl;
    std::cout << "|     Simulador de Busquedas      |" << std::endl;
    std::cout << "|                                 |" << std::endl;
    std::cout << "|  1. Generar nuevo grafo         |" << std::endl;
    std::cout << "|  2. Eliminar nodos              |" << std::endl;
    std::cout << "|  3. Realizar BFS                |" << std::endl;
    std::cout << "|  4. Realizar DFS                |" << std::endl;
    std::cout << "|  5. Realizar Hill Climbing      |" << std::endl;
    std::cout << "|  6. Realizar A*                 |" << std::endl;
    std::cout << "|  7. Salir                       |" << std::endl;
    std::cout << "===================================" << std::endl;
}

int main(int argc,char* argv[]){
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"Busquedas IA",NULL,NULL);
	if(!window){
		std::cout << " Fallo en crear la ventana :/ " << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	if(!gladLoadGL(glfwGetProcAddress)){
		std::cout << " Fallo en cargar GLAD :/ " << std::endl;
		return -1;
	}
	
	
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetKeyCallback(window,key_callback);
	glfwSetMouseButtonCallback(window,mouse_button_callback);
	
	setup_shaders(vertex_shader_id,fragment_shader_id,program_id);
	
	grafito.Fill_Graph(vertices,offset_amount,offset_by_node);
	old_vertices=vertices;
	old_offset=offset_amount;
	old_offset_by_node=offset_by_node;
	Set_Vs(vertices,VAO,VBO);
	
	print_menu();
	
	while(!glfwWindowShouldClose(window)){
		int fb_width, fb_height;
		glfwGetFramebufferSize(window, &fb_width, &fb_height);
		if (fb_width == 0 || fb_height == 0) {
			glfwWaitEvents(); 
			continue;
		}
		
		glClearColor(1.0f,1.0f,1.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(program_id);
		
		GLint screenCordLoc= glGetUniformLocation(program_id,"screenCords");
		if (screenCordLoc != -1) {
			glUniform2f(screenCordLoc, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
		}
		else {
			std::cout << "No se encontro screenCords" << std::endl;
		}
		

		GLint escalaLoc= glGetUniformLocation(program_id,"escala");
		if (escalaLoc != -1) {
			glUniform1f(escalaLoc, zoom);
		}
		else {
			std::cout << "No se encontro escala" << std::endl;
		}
		
		glBindVertexArray(VAO);
		glLineWidth(4.0f);
		glDrawArrays(GL_LINES,0,vertices.size());
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
	glDeleteProgram(program_id);
	
	glfwTerminate();
	return 0;
}

