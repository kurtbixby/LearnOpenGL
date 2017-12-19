#include "Shader.h"

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const GLchar* vertex_shader_path, const GLchar* fragment_shader_path)
{
    const unsigned int vertex_shader = CompileShader(vertex_shader_path, GL_VERTEX_SHADER);
    const unsigned int fragment_shader = CompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);
    
    // Create Program
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    id_ = shader_program;
}

unsigned int Shader::CompileShader(const GLchar* shader_path, const GLenum shader_type)
{
    // Get source code
    std::string shader_code;
    std::ifstream shader_file;

    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shader_file.open(shader_path);

        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();

        shader_file.close();

        shader_code = shader_stream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        std::cout << "SHADER PATH: " << shader_path << std::endl;
    }

    const char* shader_code_c = shader_code.c_str();


    // Compile
    unsigned int shader_id;
    int success;
    const int log_size = 512;
    char info_log[log_size];

    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_code_c, 0);
    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (success)
    {
        std::cout << "SHADER COMPILATION SUCCESSFUL" << std::endl;
    }
    if (!success)
    {
        glGetShaderInfoLog(shader_id, log_size, nullptr, info_log);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
    }

    return shader_id;
}
