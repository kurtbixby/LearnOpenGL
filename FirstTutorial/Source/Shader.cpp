#include "Headers/Shader.h"

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

Shader::Shader(const GLchar* vertex_shader_path, const GLchar* geometry_shader_path, const GLchar* fragment_shader_path)
{
    const unsigned int vertex_shader = CompileShader(vertex_shader_path, GL_VERTEX_SHADER);
    const unsigned int geometry_shader = CompileShader(geometry_shader_path, GL_GEOMETRY_SHADER);
    const unsigned int fragment_shader = CompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);
    
    // Create Program
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, geometry_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);
    
    id_ = shader_program;
}

unsigned int Shader::CompileShader(const GLchar* shader_path, const GLenum shader_type)
{
    std::cout << "COMPILING SHADER: " << shader_path << std::endl;
    
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

void Shader::AddDirectionLight(const Light& light, const int index)
{
    std::string lightName = "lights[";
    lightName.append(std::to_string(index)).append("]");
    SetVec3((lightName + std::string(".direction")).c_str(), light.data_.direction.x, light.data_.direction.y, light.data_.direction.z);
    SetVec3((lightName + std::string(".ambient")).c_str(), light.data_.color_data.ambient.x, light.data_.color_data.ambient.y, light.data_.color_data.ambient.z);
    SetVec3((lightName + std::string(".diffuse")).c_str(), light.data_.color_data.diffuse.x, light.data_.color_data.diffuse.y, light.data_.color_data.diffuse.z);
    SetVec3((lightName + std::string(".specular")).c_str(), light.data_.color_data.specular.x, light.data_.color_data.specular.y, light.data_.color_data.specular.z);
}

void Shader::AddPointLight(const PointLight& pointLight, const int index)
{
    std::string lightName = "pointLights[";
    lightName.append(std::to_string(index)).append("]");
    SetVec3((lightName + std::string(".position")).c_str(), pointLight.data_.position.x, pointLight.data_.position.y, pointLight.data_.position.z);
    SetVec3((lightName + std::string(".ambient")).c_str(), pointLight.data_.color_data.ambient.x, pointLight.data_.color_data.ambient.y, pointLight.data_.color_data.ambient.z);
    SetVec3((lightName + std::string(".diffuse")).c_str(), pointLight.data_.color_data.diffuse.x, pointLight.data_.color_data.diffuse.y, pointLight.data_.color_data.diffuse.z);
    SetVec3((lightName + std::string(".specular")).c_str(), pointLight.data_.color_data.specular.x, pointLight.data_.color_data.specular.y, pointLight.data_.color_data.specular.z);
    SetFloat((lightName + std::string(".constant")).c_str(),  pointLight.data_.constant);
    SetFloat((lightName + std::string(".linear")).c_str(),    pointLight.data_.linear);
    SetFloat((lightName + std::string(".quadratic")).c_str(), pointLight.data_.quadratic);
}

void Shader::AddSpotLight(const SpotLight& spotLight, const int index)
{
    std::string lightName = "spotLights[";
    lightName.append(std::to_string(index)).append("]");
    SetVec3((lightName + std::string(".position")).c_str(), spotLight.data_.position.x, spotLight.data_.position.y, spotLight.data_.position.z);
    SetVec3((lightName + std::string(".direction")).c_str(), spotLight.data_.direction.x, spotLight.data_.direction.y, spotLight.data_.direction.z);
    SetVec3((lightName + std::string(".ambient")).c_str(), spotLight.data_.color_data.ambient.x, spotLight.data_.color_data.ambient.y, spotLight.data_.color_data.ambient.z);
    SetVec3((lightName + std::string(".diffuse")).c_str(), spotLight.data_.color_data.diffuse.x, spotLight.data_.color_data.diffuse.y, spotLight.data_.color_data.diffuse.z);
    SetVec3((lightName + std::string(".specular")).c_str(), spotLight.data_.color_data.specular.x, spotLight.data_.color_data.specular.y, spotLight.data_.color_data.specular.z);
    SetFloat((lightName + std::string(".innerCutoff")).c_str(), spotLight.data_.innerCutoff);
    SetFloat((lightName + std::string(".outerCutoff")).c_str(), spotLight.data_.outerCutoff);
}
