#ifndef GRAPHER_H
#define GRAPHER_H

/*
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
*/

#define PI 3.14159265359

#include <vector>
#include <string>
#include <cmath>

float get_angle(const float& in_angle)
{ return (in_angle * PI / 180.0f); }

class Grapher
{
    public:
    float step, init_x, init_y;

    Grapher():
        step(0), init_x(0), init_y(0), n(0), VAO_TABLE(0), VBO_TABLE(0), init(0.2f)
    { }

    void set_grapher(int in_n)
    {
        n = in_n;

        vertices_table.clear();

        step = (2.0f - (2 * init)) / float(n);

        init_x = -1.0f + init;
        float end_x = 1.0f - init;

        init_y = 1.0f - init;
        float end_y = -1.0f + init;

        
        for (int y = 0; y <= n; y++)
        {
            float current_y = init_y - (step * y);

            vertices_table.push_back(init_x);
            vertices_table.push_back(current_y);
            vertices_table.push_back(0.0f);

            vertices_table.push_back(end_x);
            vertices_table.push_back(current_y);
            vertices_table.push_back(0.0f);
        }

        for (int x = 0; x <= n; x++)
        {
            float current_x = init_x + (step * x);

            vertices_table.push_back(current_x);
            vertices_table.push_back(init_y);
            vertices_table.push_back(0.0f);

            vertices_table.push_back(current_x);
            vertices_table.push_back(end_y);
            vertices_table.push_back(0.0f);
        }
    }

    void set_table()
    {
        if (VAO_TABLE == 0)
        {
            glGenVertexArrays(1, &VAO_TABLE);
            glGenBuffers(1, &VBO_TABLE);
        }
        
        glBindVertexArray(VAO_TABLE);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO_TABLE);

        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_table.size(), vertices_table.data(), GL_STATIC_DRAW);
        
        
        // Specify how to read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Unbind - optional for security
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void draw_table()
    {
        glBindVertexArray(VAO_TABLE);
        glDrawArrays(GL_LINES, 0, vertices_table.size());
    }

    void set_circle()
    {
        vertices_circle.clear();

        int n_points = 100;
        float radius = 1.0;

        float step_circle = 360.0 / float(n_points);

        vertices_circle.push_back(0.0f);
        vertices_circle.push_back(0.0f);
        vertices_circle.push_back(0.0f);
        

        for (int i = 0; i <= n_points; i++)
        {
            float angle = get_angle(step_circle * i);
            float x = 0.0 + radius * std::cos(angle);
            float y = 0.0 + radius * std::sin(angle);

            vertices_circle.push_back(x);
            vertices_circle.push_back(y);
            vertices_circle.push_back(0.0);
        }


        glGenVertexArrays(1, &VAO_CIRCLES);
        glGenBuffers(1, &VBO_CIRCLES);
        
        glBindVertexArray(VAO_CIRCLES);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO_CIRCLES);

        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_circle.size(), vertices_circle.data(), GL_STATIC_DRAW);
        
        
        // Specify how to read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Unbind - optional for security
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void draw_circle()
    {
        glBindVertexArray(VAO_CIRCLES);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertices_circle.size());
    }

    void set_cross()
    {
        vertices_crosses = {0.0f, 0.0f, 0.0f, // CENTRO

                            -1.0f, 0.8f, 0.0f,
                            -0.8f, 1.0f, 0.0f,
                            0.0f, 0.2f, 0.0f,
                            0.8f, 1.0f, 0.0f,
                            1.0f, 0.8f, 0.0f,
                            0.2f, 0.0f, 0.0f,
                            1.0f, -0.8f, 0.0f,
                            0.8f, -1.0f, 0.0f,
                            0.0f, -0.2f, 0.0f,
                            -0.8f, -1.0f, 0.0f,
                            -1.0f, -0.8f, 0.0f,
                            -0.2f, 0.0f, 0.0f };
        
        indices_crosses = {0,
                           12, 1, 1, 2, 2, 3, 3,4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12 };
        
        glGenVertexArrays(1, &VAO_CROSSES);
        glGenBuffers(1, &VBO_CROSSES);
        glGenBuffers(1, &EBO_CROSSES);

        glBindVertexArray(VAO_CROSSES);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_CROSSES);        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_crosses.size(), vertices_crosses.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_CROSSES);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_crosses.size(), indices_crosses.data(), GL_DYNAMIC_DRAW);
        
        // Specify how to read
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Unbind - optional for security
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void draw_cross()
    {
        glBindVertexArray(VAO_CROSSES);
        glDrawElements(GL_TRIANGLE_FAN, vertices_crosses.size(), GL_UNSIGNED_INT, (void*)0);
    }

private:
    unsigned int VAO_TABLE, VBO_TABLE,
                 VAO_CIRCLES, VBO_CIRCLES,
                 VAO_CROSSES, VBO_CROSSES, EBO_CROSSES;
    int n;
    float init;
    std::vector<float> vertices_table;
    std::vector<float> vertices_circle;
    std::vector<float> vertices_crosses;
    std::vector<unsigned int> indices_crosses;
};


#endif