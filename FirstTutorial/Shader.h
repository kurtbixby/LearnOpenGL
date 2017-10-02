#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // Constructor
    Shader(const GLchar* vertex_shader_path, const GLchar* fragment_shader_path);

    // Activate the shader
    void Use();

    // Utility uniform methods
    void SetBool(const std::string &name, const bool value) const;
    void SetInt(const std::string &name, const int value) const;
	void SetFloat(const std::string& name, const float value) const;
    void SetFloat4(const std::string& name, const float value1, const float value2, const float value3, const float value4) const;

private:
    // Shader Program ID
    unsigned int id_;

    static unsigned int CompileShader(const GLchar* shader_path, const GLenum shader_type);
};

inline Shader::Shader(const GLchar* vertex_shader_path, const GLchar* fragment_shader_path)
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

inline void Shader::Use()
{
    glUseProgram(id_);
}

inline void Shader::SetBool(const std::string& name, const bool value) const
{
    glUniform1i(glGetUniformLocation(id_, name.c_str()), static_cast<int>(value));
}

inline void Shader::SetInt(const std::string& name, const int value) const
{
    glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

inline void Shader::SetFloat(const std::string& name, const float value) const
{
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

inline void Shader::SetFloat4(const std::string& name, const float value1, const float value2, const float value3, const float value4) const
{
    glUniform4f(glGetUniformLocation(id_, name.c_str()), value1, value2, value3, value4);
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


#endif // SHADER_H
