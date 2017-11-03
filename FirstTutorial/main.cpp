#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <string>

#define _USE_MATH_DEFINES
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "Lights.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int create_window(GLFWwindow** foo);
void process_input(GLFWwindow* window, Camera& cam);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void check_shader_compilation(unsigned int shader);
void check_shader_program_linking(unsigned int program);
unsigned int create_shader(GLenum shader_type, const char* source);
unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type = GL_REPEAT);

void generate_cube_locations(const unsigned number, glm::vec3* const cube_array);
void send_direction_light(const Shader& shader, const Light& light, const int index);
void send_point_light(const Shader& shader, const PointLight& pointLight, const int index);
void send_spot_light(const Shader& shader, const SpotLight& spotLight, const int index);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
const unsigned int FOV = 45;
float texture_mix = 0.5f;
float mix_increment = 0.01f;
float mix_min = 0.0f;
float mix_max = 1.0f;

float current_x = WIDTH / 2.0f;
float current_y = HEIGHT / 2.0f;

float last_x = current_x;
float last_y = current_y;

float y_offset = 0.0f;

int main()
{
	GLFWwindow* window;
	if (create_window(&window))
	{
		std::cout << "Error creating window" << std::endl;
		return -1;
	}

	boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/DiffuseMap.vert").make_preferred();
	boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/MultipleLights.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());

	boost::filesystem::path light_vertex_shader_path = boost::filesystem::path("Shaders/BasicColor.vert").make_preferred();
	boost::filesystem::path light_fragment_shader_path = boost::filesystem::path("Shaders/Light.frag").make_preferred();
	Shader lamp_shader = Shader(light_vertex_shader_path.string().c_str(), light_fragment_shader_path.string().c_str());

    // vertex data
	float vertices[] = {
	    // positions          // normals           // texture coords
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};
    // create a Vertex Buffer Object to hold the vertices
    // the ID of the VBO is 1
    unsigned int vbo;
    glGenBuffers(1, &vbo);

	// Create Vertex Array and related buffers
	unsigned int vao;
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // sets VBO to the current GL buffer array
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sends data from to the buffer
	
	// Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	unsigned int lightVao;
	glGenVertexArrays(1, &lightVao);
	glBindVertexArray(lightVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	// Texture Loading
	boost::filesystem::path diffuse_texture_path = boost::filesystem::path("resources/container2_diffuse.png").make_preferred();
	unsigned int diffuse_texture = load_texture(diffuse_texture_path.string().c_str(), GL_RGBA);
    boost::filesystem::path specular_texture_path = boost::filesystem::path("resources/container2_specular.png").make_preferred();
    unsigned int specular_texture = load_texture(specular_texture_path.string().c_str(), GL_RGBA);
    // boost::filesystem::path texture1_path = boost::filesystem::path("resources/awesomeface.png").make_preferred();
	// unsigned int texture1 = load_texture(texture1_path.string().c_str(), GL_RGBA);

	// Cube generation
	const unsigned int cubes = 10;
	glm::vec3 cube_locations[cubes];
	generate_cube_locations(cubes, cube_locations);

	// Camera initialization
	Camera cam = Camera();
	cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));

	// For the moving light
	glm::vec3 lightDirection(-12.0f, -10.0f, 20.0f);
	double lightDeltaTheta = M_PI_4 / 1000.0f;
	glm::mat3 lightMultiplication(1.0f);
	lightMultiplication[0][0] = cos(lightDeltaTheta);
	lightMultiplication[0][2] = -sin(lightDeltaTheta);
	lightMultiplication[2][0] = sin(lightDeltaTheta);
	lightMultiplication[2][2] = cos(lightDeltaTheta);

    Light lights[NUM_DIR_LIGHTS];
    PointLight pointLights[NUM_POINT_LIGHTS];
    SpotLight spotLights[NUM_SPOT_LIGHTS];

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        process_input(window, cam);

		glm::vec3 camPosition = cam.GetPosition();
        glm::vec3 camDirection = cam.GetDirection();

        // render section of main loop
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setter
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user

		glm::mat4 projection = cam.GetProjection();
		glm::mat4 view = cam.MakeViewMat();
        // Uncomment to have the light orbit
		// lightDirection = lightMultiplication * lightDirection;

		glm::vec3 redLight(1.0f, 0.0f, 0.0f);
        glm::vec3 greenLight(0.0f, 1.0f, 0.0f);
        glm::vec3 blueLight(1.0f, 1.0f, 1.0f);
        // Uncomment for changing color
		// double time = glfwGetTime();
		// lightColor.x = sin(time * 2.0f);
		// lightColor.y = sin(time * 0.7f);
		// lightColor.z = sin(time * 1.3f);

		glm::vec3 redDiffuse = redLight * 0.5f;
		glm::vec3 redAmbient = redLight * 0.2f;
        glm::vec3 greenDiffuse = greenLight * 0.5f;
        glm::vec3 greenAmbient = greenLight * 0.2f;
        glm::vec3 blueDiffuse = blueLight * 0.5f;
        glm::vec3 blueAmbient = blueLight * 0.2f;

        lights[0].direction = glm::vec3(lightDirection.x, lightDirection.y, lightDirection.z);
        lights[0].ambient = glm::vec3(redAmbient.x, redAmbient.y, redAmbient.z);
        lights[0].diffuse = glm::vec3(redDiffuse.x, redDiffuse.y, redDiffuse.z);
        lights[0].specular = glm::vec3(1.0f);

        pointLights[0].position = glm::vec3(0.0f);
        pointLights[0].ambient = glm::vec3(greenAmbient.x, greenAmbient.y, greenAmbient.z);
        pointLights[0].diffuse = glm::vec3(greenDiffuse.x, greenDiffuse.y, greenDiffuse.z);
        pointLights[0].specular = glm::vec3(1.0f);
        pointLights[0].constant = 1.0f;
        pointLights[0].linear = 0.09f;
        pointLights[0].quadratic = 0.032f;

        spotLights[0].position = glm::vec3(camPosition.x, camPosition.y, camPosition.z);
        spotLights[0].direction = glm::vec3(camDirection.x, camDirection.y, camDirection.z);
        spotLights[0].ambient = glm::vec3(blueAmbient.x, blueAmbient.y, blueAmbient.z);
        spotLights[0].diffuse = glm::vec3(blueDiffuse.x, blueDiffuse.y, blueDiffuse.z);
        spotLights[0].specular = glm::vec3(1.0f);
        spotLights[0].innerCutoff = cos(glm::radians(10.0f));
        spotLights[0].outerCutoff = cos(glm::radians(15.0f));

        standard_shader.Use();
        standard_shader.SetMatrix4fv("projection", glm::value_ptr(projection));
        standard_shader.SetMatrix4fv("view", glm::value_ptr(view));
		standard_shader.SetVec3("viewPos", camPosition.x, camPosition.y, camPosition.z);

        for (int i = 0; i < NUM_DIR_LIGHTS; i++)
        {
            Light* light = &lights[i];
            send_direction_light(standard_shader, *light, i);
        }

        for (int i = 0; i < NUM_POINT_LIGHTS; i++)
        {
            PointLight* light = &pointLights[i];
            send_point_light(standard_shader, *light, i);
        }

        for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
        {
            SpotLight* light = &spotLights[i];
            send_spot_light(standard_shader, *light, i);
        }

		// Texture area
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);
		standard_shader.SetInt("material.diffuse", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_texture);
		standard_shader.SetInt("material.specular", 1);
        standard_shader.SetFloat("material.shininess", 32.0f);

		glBindVertexArray(vao);
		for (int i = 0; i < cubes; i++)
		{
			glm::mat4 model(1.0f);

			model = glm::translate(model, cube_locations[i]);
			//model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(50.0f) + i, glm::vec3(0.5f, 1.0f, 0.0f));
			standard_shader.SetMatrix4fv("model", glm::value_ptr(model));

			// Do the transpose on main CPU rather than GPU, expensive operation
			// Everything is done in view space
			glm::mat4 normal(1.0f);
			normal = glm::transpose(glm::inverse(view * model));
			standard_shader.SetMatrix4fv("normal", glm::value_ptr(normal));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindVertexArray(lightVao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

int create_window(GLFWwindow** foo)
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad load all opengl fxn pointers
	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	*foo = window;

	return 0;
}

void process_input(GLFWwindow* window, Camera& cam)
{
	CameraInput input = CameraInput();
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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		input.MoveForward = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		input.MoveBack = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		input.MoveRight = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		input.MoveLeft = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		input.RotateRight = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		input.RotateLeft = 1;
	}
	
	float x_delta = current_x - last_x;
	float y_delta = last_y - current_y;

	last_x = current_x;
	last_y = current_y;

	input.x_delta = x_delta;
	input.y_delta = y_delta;

	input.y_offset = y_offset;
	y_offset = 0.0f;

	cam.TakeInput(input);
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	current_x = xPos;
	current_y = yPos;
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	y_offset += yOffset;
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

void generate_cube_locations(const unsigned number, glm::vec3* const cube_array)
{
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

	for (int i = 0; i < number; i++)
	{
		const float x = (((rand() % random_parts) / static_cast<float>(random_parts)) * x_range) + x_min;
		const float y = (((rand() % random_parts) / static_cast<float>(random_parts)) * y_range) + y_min;
		const float z = (((rand() % random_parts) / static_cast<float>(random_parts)) * z_range) + z_min;

		cube_array[i] = glm::vec3(x, y, z);
	}
}

void send_direction_light(const Shader& shader, const Light& light, const int index)
{
    std::string lightName = "lights[";
    lightName.append(std::to_string(index)).append("]");
    shader.SetVec3((lightName + std::string(".direction")).c_str(), light.direction.x, light.direction.y, light.direction.z);
    shader.SetVec3((lightName + std::string(".ambient")).c_str(), light.ambient.x, light.ambient.y, light.ambient.z);
    shader.SetVec3((lightName + std::string(".diffuse")).c_str(), light.diffuse.x, light.diffuse.y, light.diffuse.z);
    shader.SetVec3((lightName + std::string(".specular")).c_str(), light.specular.x, light.specular.y, light.specular.z);
}

void send_point_light(const Shader& shader, const PointLight& pointLight, const int index)
{
    std::string lightName = "pointLights[";
    lightName.append(std::to_string(index)).append("]");
    shader.SetVec3((lightName + std::string(".position")).c_str(), pointLight.position.x, pointLight.position.y, pointLight.position.z);
    shader.SetVec3((lightName + std::string(".ambient")).c_str(), pointLight.ambient.x, pointLight.ambient.y, pointLight.ambient.z);
    shader.SetVec3((lightName + std::string(".diffuse")).c_str(), pointLight.diffuse.x, pointLight.diffuse.y, pointLight.diffuse.z);
    shader.SetVec3((lightName + std::string(".specular")).c_str(), pointLight.specular.x, pointLight.specular.y, pointLight.specular.z);
    shader.SetFloat((lightName + std::string(".constant")).c_str(),  pointLight.constant);
    shader.SetFloat((lightName + std::string(".linear")).c_str(),    pointLight.linear);
    shader.SetFloat((lightName + std::string(".quadratic")).c_str(), pointLight.quadratic);
}

void send_spot_light(const Shader& shader, const SpotLight& spotLight, const int index)
{
    std::string lightName = "spotLights[";
    lightName.append(std::to_string(index)).append("]");
    shader.SetVec3((lightName + std::string(".position")).c_str(), spotLight.position.x, spotLight.position.y, spotLight.position.z);
    shader.SetVec3((lightName + std::string(".direction")).c_str(), spotLight.direction.x, spotLight.direction.y, spotLight.direction.z);
    shader.SetVec3((lightName + std::string(".ambient")).c_str(), spotLight.ambient.x, spotLight.ambient.y, spotLight.ambient.z);
    shader.SetVec3((lightName + std::string(".diffuse")).c_str(), spotLight.diffuse.x, spotLight.diffuse.y, spotLight.diffuse.z);
    shader.SetVec3((lightName + std::string(".specular")).c_str(), spotLight.specular.x, spotLight.specular.y, spotLight.specular.z);
    shader.SetFloat((lightName + std::string(".innerCutoff")).c_str(), spotLight.innerCutoff);
    shader.SetFloat((lightName + std::string(".outerCutoff")).c_str(), spotLight.outerCutoff);
}