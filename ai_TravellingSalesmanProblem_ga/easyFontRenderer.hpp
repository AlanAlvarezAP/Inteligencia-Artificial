#ifndef EASY_FONT_RENDERER_HPP
#define EASY_FONT_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <vector>

using namespace std;

struct V4
{
	float x, y, z, w;
};

class EasyFontRenderer
{
public:
    unsigned int VAO = 0, VBO = 0, prog = 0;
 
    void init();
    void draw(const char* text, float x, float y, float scale,
		float r, float g, float b, float vpW, float vpH);
    void cleanup();
};

#endif