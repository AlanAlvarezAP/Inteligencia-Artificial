#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

#define GLAD_GL_IMPLEMENTATION
#include "easyFontRenderer.hpp"

// txt shaders
const char *textVertSrc = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform vec2 uResolution;\n"
    "uniform vec2 uOffset;\n"
    "void main() {\n"
    "    vec2 p = (aPos + uOffset) / uResolution * 2.0 - 1.0;\n"
    "    p.y = -p.y;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}\0";
 
const char *textFragSrc = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    FragColor = vec4(uColor, 1.0);\n"
    "}\0";

void EasyFontRenderer::init()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// space for text
	glBufferData(GL_ARRAY_BUFFER, 1024 * 16 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	// use for xy only
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	auto comp = [](GLenum t, const char* src)
	{
		unsigned int s = glCreateShader(t);
		glShaderSource(s, 1, &src, NULL);
		glCompileShader(s);
		return s;
	};
	unsigned int vs = comp(GL_VERTEX_SHADER, textVertSrc);
	unsigned int fs = comp(GL_FRAGMENT_SHADER, textFragSrc);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);
}

// x y in viewport pixels
// vpW vpH - viewport dimensions
void EasyFontRenderer::draw(const char* text, float x, float y, float scale,
	float r, float g, float b, float vpW, float vpH)
{
	static char buf[1024 * 16];
	int numQuads = stb_easy_font_print(0, 0, (char*)text, NULL, buf, sizeof(buf));

	// convert quads to triangles - 2 per quad = 6 vertex
	V4* verts = (V4*)buf;

	vector<float> tris;
	tris.reserve(numQuads * 6 * 2);
	for (int q = 0; q < numQuads; q++)
	{
		V4* v = verts + q * 4;
		// tri 1
		tris.push_back(v[0].x * scale); tris.push_back(v[0].y * scale); tris.push_back(0); tris.push_back(0);
		tris.push_back(v[1].x * scale); tris.push_back(v[1].y * scale); tris.push_back(0); tris.push_back(0);
		tris.push_back(v[2].x * scale); tris.push_back(v[2].y * scale); tris.push_back(0); tris.push_back(0);
		// tri 2
		tris.push_back(v[0].x * scale); tris.push_back(v[0].y * scale); tris.push_back(0); tris.push_back(0);
		tris.push_back(v[2].x * scale); tris.push_back(v[2].y * scale); tris.push_back(0); tris.push_back(0);
		tris.push_back(v[3].x * scale); tris.push_back(v[3].y * scale); tris.push_back(0); tris.push_back(0);
	}

	glUseProgram(prog);
	glUniform2f(glGetUniformLocation(prog, "uResolution"), vpW, vpH);
	glUniform2f(glGetUniformLocation(prog, "uOffset"), x, y);
	glUniform3f(glGetUniformLocation(prog, "uColor"), r, g, b);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, tris.size()*sizeof(float), tris.data());
	glDrawArrays(GL_TRIANGLES, 0, (int)tris.size() / 4);
	glBindVertexArray(0);
}

void EasyFontRenderer::cleanup()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(prog);
}