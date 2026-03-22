#pragma once
#include "config.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#define FLOAT_MAX std::numeric_limits<float>::max()

class Node {
public:
    int index;
    float x, y,dist_dest;
    std::vector<Node*> nigh;

    Node();
    Node(int idx);
    Node(int idx, float xx, float yy);
};

class Traversal {
public:
    Node* origin;
    std::vector<Node*> path_to_origin;
    float actu_trav;

    struct Hill_Climb_Comp {
        bool operator()(const Traversal &first,const Traversal& other) const {
            return first.origin->dist_dest > other.origin->dist_dest;
        }
    };

    struct A_star_Comp {
        bool operator()(const Traversal& first, const Traversal& other) const {
            float comp_1 = first.origin->dist_dest + first.actu_trav;
            float comp_2 = other.origin->dist_dest + other.actu_trav;
            return comp_1 > comp_2;
        }
    };
};

class Graph {
public:
    std::vector<Node*> graph;
	std::map<std::pair<float,float>,int> get_node;
    void Fill_Graph(std::vector<Vertex> &vex,std::map<std::pair<int,int>,int> &offset_amount,std::map<std::pair<float,float>,std::vector<int>> &offset_by_node);
    void print_grade();
    void Fill_Heuristics(int end_node);
    void BFS(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window);
    void DFS(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window);
    void Hillclimbing(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window);
    void A_star(int start_index, int end_index,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO,GLFWwindow* window);
    void Eliminate_random(std::vector<Vertex>& vertices, std::map<std::pair<int,int>, int>& offset_amount,std::map<std::pair<float,float>,std::vector<int>> &offset_by_node);
};