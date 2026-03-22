#pragma once
#include "config.h"
#include <iostream>
#include <vector>
#include <map>
#include "Graph.h"


void updateColorNode(std::pair<float, float> index_node, float r, float g, float b,std::vector<Vertex>& vertices,std::map<std::pair<float, float>, std::vector<int>>& offset_by_node,unsigned int VBO);
