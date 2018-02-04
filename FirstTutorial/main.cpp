#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <string>

#define _USE_MATH_DEFINES
#include <cmath>

#include "Camera.h"
#include "Framebuffer.h"
#include "Model.h"
#include "Primitives.h"
#include "Scene.h"
#include "SceneGraph.h"
#include "Shader.h"
#include "Structs.h"

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ModelLoader.h"

int create_window(GLFWwindow** foo);
Input get_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

void generate_cube_locations(const unsigned number, glm::vec3* const cube_array);
Scene load_scene();

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

//unordered_map<string, Model> loaded_models_ = unordered_map<string, Model>();

int main()
{
	GLFWwindow* window;
	if (create_window(&window))
	{
		std::cout << "Error creating window" << std::endl;
		return -1;
	}
	
	Scene scene = load_scene();

	// Create standard shader
	boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/Screen.vert").make_preferred();
	boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/Kernel.frag").make_preferred();
	Shader screenShader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());

	// Create intermediary framebuffer
	Framebuffer fbuffer = Framebuffer();
	fbuffer.AddTextureAttachment(FBAttachment::Color);
	fbuffer.AddRenderbufferAttachment(FBAttachment::DepthStencil);

	// Create Screen Quad VAO
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO;
	glGenVertexArrays(1, &quadVAO);
	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2));
	glBindVertexArray(0);

	// Edge Detection?
	/*float kernel[9] = {
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	};*/

	// Identity
	float kernel[9] = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	};

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        Input input = get_input(window);
        scene.TakeInput(input);

		// Enable texture framebuffer
		fbuffer.Use();

		// (Re)enable depth for main scene
		glEnable(GL_DEPTH_TEST);
        scene.Render();
		// Disable depth for screen quad
		glDisable(GL_DEPTH_TEST);

		// Reenable default framebuffer
		Framebuffer::UseDefault();
		// Reset clears
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Enable screen shader
		screenShader.Use();

		// bind screen quad
		glBindVertexArray(quadVAO);

		glActiveTexture(GL_TEXTURE0);
		// bind fb texture
		glBindTexture(GL_TEXTURE_2D, fbuffer.RetrieveColorBuffer(0).TargetName);
		// send kernel to shader
		screenShader.SetFloats("kernel", 9, &kernel[0]);
		// draw screen quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
        // break;

		glBindTexture(GL_TEXTURE_2D, 0);
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
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    // glDepthFunc(GL_GREATER);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	*foo = window;

	return 0;
}

Input get_input(GLFWwindow* window)
{
	CameraInput cameraInput = CameraInput();
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
		cameraInput.MoveForward = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraInput.MoveBack = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraInput.MoveRight = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraInput.MoveLeft = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		cameraInput.RotateRight = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		cameraInput.RotateLeft = 1;
	}
	
	float x_delta = current_x - last_x;
	float y_delta = last_y - current_y;

	last_x = current_x;
	last_y = current_y;

	cameraInput.x_delta = x_delta;
	cameraInput.y_delta = y_delta;

	cameraInput.y_offset = y_offset;
	y_offset = 0.0f;

    Input input = Input();
    input.cameraInput = cameraInput;

    return input;
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

Scene load_scene()
{
    SceneGraph graph = SceneGraph();

    // Camera initialization
    Camera cam = Camera();
    cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));
    std::vector<Camera> cams = std::vector<Camera>();
    cams.push_back(cam);

    std::vector<Model> models = std::vector<Model>();
	ModelLoader crysisLoader = ModelLoader(boost::filesystem::path("Resources/nanosuit_reflection").make_preferred());
	Model crysis = crysisLoader.loadModel("nanosuit.obj");
	models.push_back(create_box());
	models.push_back(create_plane());
	models.push_back(create_quad());
	models.push_back(crysis);

    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTextures.vert").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());

	boost::filesystem::path transparent_fragment_shader_path = boost::filesystem::path("Shaders/Transparency_Blended.frag").make_preferred();
	Shader transparent_shader = Shader(vertex_shader_path.string().c_str(), transparent_fragment_shader_path.string().c_str());

    boost::filesystem::path outline_fragment_shader_path = boost::filesystem::path("Shaders/Outline.frag").make_preferred();
    Shader outline_shader = Shader(vertex_shader_path.string().c_str(), outline_fragment_shader_path.string().c_str());

	boost::filesystem::path skybox_vertex_shader_path = boost::filesystem::path("Shaders/SkyboxBuff.vert").make_preferred();
	boost::filesystem::path skybox_fragment_shader_path = boost::filesystem::path("Shaders/Skybox.frag").make_preferred();
	Shader skybox_shader = Shader(skybox_vertex_shader_path.string().c_str(), skybox_fragment_shader_path.string().c_str());

    std::vector<Shader> shaders = std::vector<Shader>();
    shaders.push_back(standard_shader);
	shaders.push_back(transparent_shader);
    shaders.push_back(outline_shader);
	shaders.push_back(skybox_shader);

	Cubemap skybox = Cubemap();

    Scene scene = Scene(graph, cams, models, shaders, skybox);

    return scene;
}
