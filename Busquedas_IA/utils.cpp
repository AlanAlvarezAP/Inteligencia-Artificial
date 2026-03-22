#include "utils.h"
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



void updateColorNode(std::pair<float, float> index_node, float r, float g, float b,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    for (int vertexIdx : offset_by_node[{index_node.first, index_node.second}]) {
        vertices[vertexIdx].r = r;
        vertices[vertexIdx].g = g;
        vertices[vertexIdx].b = b;
        
        int offset = vertexIdx * sizeof(Vertex) + (3 * sizeof(float));
        float color[3] = {r, g, b};
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(color), color);
    }
}