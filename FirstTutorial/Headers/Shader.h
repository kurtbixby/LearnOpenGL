#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <iostream>

#include "Lights.h"

class Shader
{
public:
    // Constructor
    Shader();
    Shader(const GLchar* vertex_shader_path, const GLchar* fragment_shader_path);
    Shader(const GLchar* vertex_shader_path, const GLchar* geometry_shader_path, const GLchar* fragment_shader_path);

    // Activate the shader
    void Use();

    // Utility uniform methods
    void SetBool(const std::string &name, const bool value) const;
    void SetInt(const std::string &name, const int value) const;
	void SetFloat(const std::string& name, const float value) const;
    void SetFloat4(const std::string& name, const float value1, const float value2, const float value3, const float value4) const;
	void SetMatrix4fv(const std::string&name, const float* values) const;
	void SetVec3(const std::string& name, const float v0, const float v1, const float v2) const;
	void SetFloats(const std::string& name, int count, const float* value) const;
	void BindUniformBlock(const std::string& name, int binding) const;

    void AddDirectionLight(const Light& light, const int index);
    void AddPointLight(const PointLight& pointLight, const int index);
    void AddSpotLight(const SpotLight& spotLight, const int index);

private:
    // Shader Program ID
    unsigned int id_;

    static unsigned int CompileShader(const GLchar* shader_path, const GLenum shader_type);
};

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
    std::cerr << "Set Int:" << name << " " << value << std::endl;
    int location_result = glGetUniformLocation(id_, name.c_str());
    std::cerr << name << " location: " << location_result << std::endl;
    glUniform1i(location_result, value);
}

inline void Shader::SetFloat(const std::string& name, const float value) const
{
	glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

inline void Shader::SetFloat4(const std::string& name, const float value1, const float value2, const float value3, const float value4) const
{
    glUniform4f(glGetUniformLocation(id_, name.c_str()), value1, value2, value3, value4);
}

inline void Shader::SetMatrix4fv(const std::string&name, const float* value) const
{
	glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, value);
}

inline void Shader::SetVec3(const std::string& name, const float v0, const float v1, const float v2) const
{
	glUniform3f(glGetUniformLocation(id_, name.c_str()), v0, v1, v2);
}

inline void Shader::SetFloats(const std::string& name, int count, const float* value) const
{
	glUniform1fv(glGetUniformLocation(id_, name.c_str()), count, value);
}

inline void Shader::BindUniformBlock(const std::string& name, int bindingIndex) const
{
	int blockIndex = glGetUniformBlockIndex(id_, name.c_str());
	glUniformBlockBinding(id_, blockIndex, bindingIndex);
}

#endif // SHADER_H
