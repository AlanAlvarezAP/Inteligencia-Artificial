#ifndef SHADER_LIST_H
#define SHADER_LIST_H

/*
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
*/

#include <vector>
#include <map>
#include <string>

class ShaderList
{
public:
    ShaderList():
        VERTEX(0), shader_programs() {}


    void create_vertex_shader(const char *vertexShaderSource)
    {
        VERTEX = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VERTEX, 1, &vertexShaderSource, nullptr);
        glCompileShader(VERTEX);
    }

    void add_fragment_shader(const std::string& shader_name, const char *fragment_Shader_Source)
    {
        unsigned int fragment_source_shader = 0;
        fragment_source_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_source_shader, 1, &fragment_Shader_Source, nullptr);
        glCompileShader(fragment_source_shader);


        unsigned int shader_program = 0;
        shader_program = glCreateProgram();
        
        glAttachShader(shader_program, VERTEX);
        glAttachShader(shader_program, fragment_source_shader);
        glLinkProgram(shader_program);
        

        glDeleteShader(fragment_source_shader);

        shader_programs[shader_name] = shader_program;
    }

    void delete_shaders()
    {
        glDeleteShader(VERTEX);
    }

    void use_shader(const std::string& in_shader)
    {
        glUseProgram(shader_programs[in_shader]);
    }

    void delete_programs()
    {
        for (auto &i: shader_programs)
            glDeleteProgram(i.second);
    }

    void set_vec2(const std::string& name, float x, float y)
    {
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        int uniform = glGetUniformLocation(currentProgram, name.c_str());
        glUniform2f(uniform, x, y);
    }

    void set_float(const std::string& name, float val)
    {
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

        int uniform = glGetUniformLocation(currentProgram, name.c_str());

        glUniform1f(uniform, val);
    }


private:
    unsigned int VERTEX;
    std::map<std::string, unsigned int> shader_programs;
};


#endif