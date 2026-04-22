#include "graph.hpp"

Graph::Graph() {}
	
Graph::Graph(const vector<Point>& centers)
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

float Graph::getDistance(int u, int v) const
{
	return dist[u][v];
}

int Graph::size() const
{
	return n;
}

const Point& Graph::getPoint(int i) const
{
	return pts[i];
}