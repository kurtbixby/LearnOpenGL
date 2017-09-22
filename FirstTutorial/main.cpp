#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>

#include "Shader.h"

void process_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void check_shader_compilation(unsigned int shader);
void check_shader_program_linking(unsigned int program);
unsigned int create_shader(GLenum shader_type, const char* source);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

int main()
{
    // glfw initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS compatibility

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad load all opengl fxn pointers
    //if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const char* vertex_shader_path = "Colors.vert";
    const char* fragment_shader_path = "Colors.frag";
    Shader shader = Shader(vertex_shader_path, fragment_shader_path);

    // vertex data
    float vertices[] = {
        0.0f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Create Vertex Array and related buffers
    unsigned int vao;
    glGenVertexArrays(1, &vao);

    // create a Vertex Buffer Object to hold the vertices
    // the ID of the VBO is 1
    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo); // sets VBO to the current GL buffer array
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sends data from to the buffer

    /*
        first arg is which attribute to use
            defined attr 0 to be location in vert shader
        second arg is size of each attribute
        third arg is data type
        fourth arg is normalization or not
        fifth arg is stride
        what is the last arg
    */
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    /*
        These calls can be put into the main render loop once there is more than 1 object and shader
    */

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        process_input(window);

        // render section of main loop
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setter
        glClear(GL_COLOR_BUFFER_BIT); // state user

        shader.Use();
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Draw triangles using 3 vertices from vertex array 0
        // Bind the vertex array to use for the draw calls
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

unsigned int create_shader(const GLenum shader_type, const char* source)
{
    const unsigned int shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    check_shader_compilation(shader);

    return shader;
}

void check_shader_compilation(const unsigned int shader)
{
    const int logSize = 512;
    int success;
    char infoLog[logSize];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success)
    {
        std::cout << "SHADER COMPILATION SUCCESSFUL" << std::endl;
    }
    if (!success)
    {
        glGetShaderInfoLog(shader, logSize, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void check_shader_program_linking(const unsigned int program)
{
    const int logSize = 512;
    int success;
    char infoLog[logSize];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success)
    {
        std::cout << "PROGRAM LINKING SUCCESSFUL" << std::endl;
    }
    if (!success)
    {
        glGetProgramInfoLog(program, logSize, nullptr, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}