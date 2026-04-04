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

#include "Genetico.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>
#include <string>

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
		default:
			break;
	}
}

void print_menu() {
    std::cout << "===================================" << std::endl;
    std::cout << "|        Bienvenido a             |" << std::endl;
    std::cout << "| Algoritmos geneticos maximizador|" << std::endl;
    std::cout << "|                                 |" << std::endl;
    std::cout << "|  1. Escribir función            |" << std::endl;
    std::cout << "|  2. Simular Algoritmo           |" << std::endl;
    std::cout << "|  3. Generar graficos            |" << std::endl;
    std::cout << "|  7. Salir                       |" << std::endl;
    std::cout << "===================================" << std::endl;
}


int main(int argc,char* argv[]){
	
	/*glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"Algoritmos Geneticos Matematica",NULL,NULL);
	if(!window){
		std::cout << "Failed to create window :( " << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	if(!gladLoadGL(glfwGetProcAddress)){
		std::cout << "Failed to load GLAD :( " << std::endl;
		return -1;
	}
	
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetKeyCallback(window,keyboard_callback);*/
	
	
	Genetico Alg_Gen;
	Alg_Gen.Run_Genetics(20);
	
	return 0;
}

