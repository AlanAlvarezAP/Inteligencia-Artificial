#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <cmath>

using namespace std;

struct Point
{
	float x, y;
};

class Graph
{
private:
    int n;
    vector<Point> pts;
    vector<vector<float>> dist;
 
public:
    Graph();
	
    Graph(const vector<Point>& centers);
    float getDistance(int u, int v) const;
    int size() const;
    const Point& getPoint(int i) const;
};

#endif