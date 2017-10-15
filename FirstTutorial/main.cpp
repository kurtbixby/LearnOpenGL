#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void process_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void check_shader_compilation(unsigned int shader);
void check_shader_program_linking(unsigned int program);
unsigned int create_shader(GLenum shader_type, const char* source);
unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type = GL_REPEAT);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
const unsigned int cubes = 10;

float texture_mix = 0.5f;
float mix_increment = 0.01f;
float mix_min = 0.0f;
float mix_max = 1.0f;

int main()
{
    // glfw initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS compatibility
#endif

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

	glEnable(GL_DEPTH_TEST);

	boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/SpacesMatrices.vert").make_preferred();
	boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/ControlMix.frag").make_preferred();
    Shader shader = Shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());

    // vertex data
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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


	//unsigned int indices[] = {
	//	0, 1, 3,
	//	1, 2, 3
	//};
	//unsigned int ebo;
	//glGenBuffers(1, &ebo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /*
        first arg is which attribute to use
            defined attr 0 to be location in vert shader
        second arg is size of each attribute
        third arg is data type
        fourth arg is normalization or not
        fifth arg is stride
        what is the last arg
    */
	// Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
	// Texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texture Work
	boost::filesystem::path texture0_path = boost::filesystem::path("resources/container.jpg").make_preferred();
	boost::filesystem::path texture1_path = boost::filesystem::path("resources/awesomeface.png").make_preferred();
	unsigned int texture0 = load_texture(texture0_path.c_str(), GL_RGB, GL_CLAMP_TO_EDGE);
	unsigned int texture1 = load_texture(texture1_path.c_str(), GL_RGBA);

	// End Texture Work

	const int random_parts = 2000;
	const float x_max = 5.0f;
	const float x_min = -5.0f;
	const float x_range = x_max - x_min;
	const float y_max = 5.0f;
	const float y_min = -5.0f;
	const float y_range = y_max - y_min;
	const float z_max = -1.0f;
	const float z_min = -15.0f;
	const float z_range = z_max - z_min;

	glm::vec3 cube_locations[cubes];
	for (int i = 0; i < cubes; i++)
	{
		const float x = (((rand() % random_parts) / static_cast<float>(random_parts)) * x_range) + x_min;
		const float y = (((rand() % random_parts) / static_cast<float>(random_parts)) * y_range) + y_min;
		const float z = (((rand() % random_parts) / static_cast<float>(random_parts)) * z_range) + z_min;

		cube_locations[i] = glm::vec3(x, y, z);
	}

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        process_input(window);

        // render section of main loop
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setter
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user
		
		glBindVertexArray(vao);

		shader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		shader.SetInt("texture0", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		shader.SetInt("texture1", 1);

		shader.SetFloat("mixture", texture_mix);

		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(45.0f), WIDTH / static_cast<float>(HEIGHT), 0.1f, 100.0f);
		shader.SetMatrix4fv("projection", glm::value_ptr(projection));

		glm::mat4 view(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		shader.SetMatrix4fv("view", glm::value_ptr(view));

		for (int i = 0; i < cubes; i++)
		{
			glm::mat4 model(1.0f);
			//model = glm::translate(model, glm::vec3(0.0, -0.5f, 0.0f));

			model = glm::translate(model, cube_locations[i]);
			model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(50.0f) + i, glm::vec3(0.5f, 1.0f, 0.0f));
			shader.SetMatrix4fv("model", glm::value_ptr(model));

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

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
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		texture_mix = std::min(texture_mix + mix_increment, mix_max);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		texture_mix = std::max(texture_mix - mix_increment, mix_min);
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

unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nr_channels;
	unsigned char* data = stbi_load(texture_file, &width, &height, &nr_channels, 0);

	if (data)
	{

		/*
		* 1) OpenGL object type
		* 2) Mipmap level
		* 3) Format to store texture in
		* 4) Texture width
		* 5) Texture height
		* 6) Legacy stuff [what exactly?]
		* 7) Format of source texture
		* 8) Datatype of source texture
		* 9) Actual texture data
		*/
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, source_format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	return texture;
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