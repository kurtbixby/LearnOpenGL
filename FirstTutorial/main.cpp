#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <string>

#define _USE_MATH_DEFINES
#include <cmath>

#include "Camera.h"
#include "Framebuffer.h"
#include "Lights.h"
#include "Mesh.h"
#include "Model.h"
#include "Object.h"
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
#include <glm/gtc/type_ptr.hpp>

int create_window(GLFWwindow** foo);
Input get_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void check_shader_compilation(unsigned int shader);
void check_shader_program_linking(unsigned int program);

Mesh create_box();
Mesh create_plane();
Mesh create_quad();
std::pair<int, glm::vec3> create_object(const int mesh, const glm::vec3& transform);

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

unordered_map<string, Model> loaded_models_ = unordered_map<string, Model>();

int main()
{
	GLFWwindow* window;
	if (create_window(&window))
	{
		std::cout << "Error creating window" << std::endl;
		return -1;
	}
	
	Scene scene = load_scene();

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        Input input = get_input(window);

        scene.TakeInput(input);

        // render section of main loop
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // state setter
        // glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user

        scene.Render();

        // Crysis path
		// standard_shader.SetMatrix4fv("model", glm::value_ptr(model));

		// boost::filesystem::path model_path = boost::filesystem::path("Resources/nanosuit/nanosuit.obj").make_preferred();
		// string model_path_string = model_path.string();

		// auto models_iterator = loaded_models_.find(model_path_string);
		// if (models_iterator != loaded_models_.end())
		// {
		// 	cout << model_path_string << " already loaded" << endl;
		// 	models_iterator->second.Draw(standard_shader);
		// }
		// else
		// {
		// 	cout << model_path_string << " not loaded" << endl;
		// 	Model model = Model(model_path);
		// 	loaded_models_.emplace(model_path_string, model);
		// 	model.Draw(standard_shader);
		// }

        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
        // break;
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

Mesh create_box()
{
	float vertices[] = {
		// positions          // normals           // texture coords
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f
	};

	int length = sizeof(vertices) / sizeof(vertices[0]);

	std::vector<Vertex> vertexes;

	for (int i = 0; i < length; i += 8)
	{
		glm::vec3 pos = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
		glm::vec3 norm = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
		glm::vec2 coords = glm::vec2(vertices[i + 6], vertices[i + 7]);

		vertexes.push_back(create_vertex(pos, norm, coords));
	}

	std::vector<unsigned int> indices = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35
	};

	std::vector<Texture> textures;
	Texture diffTex = load_texture("Resources", "container2_diffuse.png", TextureType::Diffuse);
	Texture specTex = load_texture("Resources", "container2_specular.png", TextureType::Specular);

	textures.push_back(diffTex);
	textures.push_back(specTex);

	return Mesh(vertexes, indices, textures);
}

Mesh create_plane()
{
    float vertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        -5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f,
        -5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,

         5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,
         5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f                            
    };

    int length = sizeof(vertices) / sizeof(vertices[0]);

    std::vector<Vertex> vertexes;

    for (int i = 0; i < length; i += 8)
    {
        glm::vec3 pos = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 norm = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        glm::vec2 coords = glm::vec2(vertices[i + 6], vertices[i + 7]);

        vertexes.push_back(create_vertex(pos, norm, coords));
    }

    std::vector<unsigned int> indices = {
        0, 1, 2,
        3, 4, 5
    };

    std::vector<Texture> textures;
    Texture diffTex = load_texture("Resources", "awesomeface.png", TextureType::Diffuse);

    textures.push_back(diffTex);

    return Mesh(vertexes, indices, textures);
}

Mesh create_quad()
{
    std::vector<Vertex> vertices = {
        create_vertex( -0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f ),
        create_vertex(  0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f ),
        create_vertex(  0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f ),
        create_vertex(  0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f ),
        create_vertex( -0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f ),
        create_vertex( -0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f )
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        3, 4, 5
    };

    std::vector<Texture> textures;
    Texture diffTex = load_texture("Resources", "blending_transparent_window.png", TextureType::Diffuse);

    textures.push_back(diffTex);

    return Mesh(vertices, indices, textures);
}

std::pair<int, glm::vec3> create_object(const int mesh, const glm::vec3& transform)
{
    return std::pair<int, glm::vec3>(mesh, transform);
}

Scene load_scene()
{
    SceneGraph graph = SceneGraph();

    // Camera initialization
    Camera cam = Camera();
    cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));
    std::vector<Camera> cams = std::vector<Camera>();
    cams.push_back(cam);

    std::vector<Mesh> meshes = std::vector<Mesh>();
    meshes.push_back(create_box());
    meshes.push_back(create_plane());
    meshes.push_back(create_quad());

    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTextures.vert").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/Transparency_Blended.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());
    boost::filesystem::path outline_fragment_shader_path = boost::filesystem::path("Shaders/Outline.frag").make_preferred();
    Shader outline_shader = Shader(vertex_shader_path.string().c_str(), outline_fragment_shader_path.string().c_str());

    std::vector<Shader> shaders = std::vector<Shader>();
    shaders.push_back(standard_shader);
    shaders.push_back(outline_shader);

    Scene scene = Scene(graph, cams, meshes, shaders);

    return scene;
}

/*

    // Light creation section

    // For the moving light
    glm::vec3 lightDirection(-12.0f, -10.0f, 20.0f);
    double lightDeltaTheta = M_PI_4 / 1000.0f;
    glm::mat3 lightMultiplication(1.0f);
    lightMultiplication[0][0] = cos(lightDeltaTheta);
    lightMultiplication[0][2] = -sin(lightDeltaTheta);
    lightMultiplication[2][0] = sin(lightDeltaTheta);
    lightMultiplication[2][2] = cos(lightDeltaTheta);

    Light lights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];

    // Uncomment to have the light orbit
    // lightDirection = lightMultiplication * lightDirection;

    int DIR_LIGHTS = 0;
    int POINT_LIGHTS = 0;
    int SPOT_LIGHTS = 0;

    glm::vec3 redLight(1.0f, 0.0f, 0.0f);
    glm::vec3 greenLight(0.0f, 1.0f, 0.0f);
    glm::vec3 blueLight(0.0f, 0.0f, 1.0f);

    glm::vec3 whiteLight(1.0f, 1.0f, 1.0f);
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

    glm::vec3 whiteDiffuse = whiteLight * 0.5f;
    glm::vec3 whiteAmbient = whiteLight * 0.2f;

    lights[DIR_LIGHTS].direction = glm::vec3(lightDirection.x, lightDirection.y, lightDirection.z);
    lights[DIR_LIGHTS].ambient = glm::vec3(whiteAmbient.x, whiteAmbient.y, whiteAmbient.z);
    lights[DIR_LIGHTS].diffuse = glm::vec3(whiteDiffuse.x, whiteDiffuse.y, whiteDiffuse.z);
    lights[DIR_LIGHTS].specular = glm::vec3(1.0f);
    DIR_LIGHTS++;

    pointLights[POINT_LIGHTS].position = glm::vec3(0.0f);
    pointLights[POINT_LIGHTS].ambient = glm::vec3(greenAmbient.x, greenAmbient.y, greenAmbient.z);
    pointLights[POINT_LIGHTS].diffuse = glm::vec3(greenDiffuse.x, greenDiffuse.y, greenDiffuse.z);
    pointLights[POINT_LIGHTS].specular = glm::vec3(1.0f);
    pointLights[POINT_LIGHTS].constant = 1.0f;
    pointLights[POINT_LIGHTS].linear = 0.09f;
    pointLights[POINT_LIGHTS].quadratic = 0.032f;
    POINT_LIGHTS++;

    pointLights[POINT_LIGHTS].position = glm::vec3(-5.0f);
    pointLights[POINT_LIGHTS].ambient = glm::vec3(greenAmbient.x, greenAmbient.y, greenAmbient.z);
    pointLights[POINT_LIGHTS].diffuse = glm::vec3(greenDiffuse.x, greenDiffuse.y, greenDiffuse.z);
    pointLights[POINT_LIGHTS].specular = glm::vec3(1.0f);
    pointLights[POINT_LIGHTS].constant = 1.0f;
    pointLights[POINT_LIGHTS].linear = 0.09f;
    pointLights[POINT_LIGHTS].quadratic = 0.032f;
    POINT_LIGHTS++;

    spotLights[SPOT_LIGHTS].position = glm::vec3(camPosition.x, camPosition.y, camPosition.z);
    spotLights[SPOT_LIGHTS].direction = glm::vec3(camDirection.x, camDirection.y, camDirection.z);
    spotLights[SPOT_LIGHTS].ambient = glm::vec3(blueAmbient.x, blueAmbient.y, blueAmbient.z);
    spotLights[SPOT_LIGHTS].diffuse = glm::vec3(blueDiffuse.x, blueDiffuse.y, blueDiffuse.z);
    spotLights[SPOT_LIGHTS].specular = glm::vec3(1.0f);
    spotLights[SPOT_LIGHTS].innerCutoff = cos(glm::radians(10.0f));
    spotLights[SPOT_LIGHTS].outerCutoff = cos(glm::radians(15.0f));
    SPOT_LIGHTS++;

    // End light creation section

*/

/*// Primitives Creation

        std::vector<Mesh> meshes = std::vector<Mesh>();
        meshes.push_back(create_box());
        meshes.push_back(create_plane());
        meshes.push_back(create_quad());

        // End Primitives Creation


        // Scene Objects Creation

        std::vector<Object> objects = std::vector<Object>();
        objects.push_back(Object(glm::vec3(0.0f), 1, 1.0f, false, true));
        objects.push_back(Object(glm::vec3(-1.0f, 0.0f, -1.0f), 0, 1.0f, false));
        objects.push_back(Object(glm::vec3(2.0f, 0.0f, 0.0f), 0, 1.0f, false));
        objects.push_back(Object(glm::vec3(-1.5f,  0.0f, -0.48f), 2, 1.0f, false, true, true));
        objects.push_back(Object(glm::vec3( 1.5f,  0.0f,  0.51f), 2, 1.0f, false, true, true));
        objects.push_back(Object(glm::vec3( 0.0f,  0.0f,  0.7f), 2, 1.0f, false, true, true));
        objects.push_back(Object(glm::vec3(-0.3f,  0.0f, -2.3f), 2, 1.0f, false, true, true));
        objects.push_back(Object(glm::vec3( 0.5f,  0.0f, -0.6f), 2, 1.0f, false, true, true));

        // End Scene Objects Creation
*/
