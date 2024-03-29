#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <chrono>
#include <cmath>

#define _USE_MATH_DEFINES
#include <cmath>

#include "Headers/Debug.h"

#include "Headers/Camera.h"
#include <Headers/CompilerWarning.h>
#include "Headers/Framebuffer.h"
#include "Headers/InputWrapper.h"
#include "Headers/Model.h"
#include "Headers/PrimitivesLoader.h"
#include "Headers/RenderConfig.h"
#include "Headers/Renderer.h"
#include "Headers/Scene.h"
#include "Headers/SceneGraph.h"
#include "Headers/ScreenRenderer.h"
#include "Headers/Shader.h"
#include "Headers/Structs.h"
#include "Headers/Timer.h"

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Headers/ModelLoader.h"

#define WIDTH 1280
#define HEIGHT 720

int create_window(GLFWwindow** foo, InputWrapper& inputWrapper, uint32_t width, uint32_t height);
Input get_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void generate_cube_locations(const unsigned number, glm::vec3* const cube_array);
Scene load_normal_scene();
Scene load_scene();

void timer_message(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end, std::string message);

int main()
{
    // Change to load config from disk
    RenderConfig config = RenderConfig(WIDTH, HEIGHT);
    
    // Create InputWrapper?
    InputWrapper inputWrapper = InputWrapper();
	GLFWwindow* window;
	if (create_window(&window, inputWrapper, config.WindowWidth(), config.WindowHeight()))
	{
		std::cerr << "Error creating window" << std::endl;
		return -1;
	}
    Scene scene = load_scene();
//    Scene scene = load_normal_scene();
    
//    if (config.ShadowmapsEnabled())
//    {
//        // Switch this to use SceneRenderer
//        Framebuffer shadow_map_buffer = Framebuffer(SHADOW_RES, SHADOW_RES, 1, false);
//        scene.GenerateShadowMaps(shadow_map_buffer);
//    }
    
    ScreenRenderer scrRenderer = ScreenRenderer(config);
    SceneRenderer sceneRenderer = SceneRenderer(&scene, &scrRenderer, &config);
    Renderer renderer = Renderer(config, &scrRenderer);
    
    Timer frame_timer = Timer();
    // main loop
    while (!glfwWindowShouldClose(window))
    {
        frame_timer.Start();
        // process any input
        Input input = inputWrapper.TakeInput(window);
        if (input.ChangeRenderMethod())
        {
            renderer.SwitchRenderMethod();
            sceneRenderer.RenderChanged();
        }
        scene.TakeInput(input);
        
        scene.Simulate();
        
        renderer.RenderScene(sceneRenderer);
        
        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
        // break;

//        glBindTexture(GL_TEXTURE_2D, 0);
        frame_timer.Stop();
        std::cout << "Frame time: " << frame_timer.ElapsedTime() << std::endl;
        frame_timer.Reset();
    }

    glfwTerminate();
    return 0;
}

int create_window(GLFWwindow** foo, InputWrapper& inputWrapper, uint32_t width, uint32_t height)
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
	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", nullptr, nullptr);
	if (nullptr == window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, &inputWrapper);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

	// glad load all opengl fxn pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
//    glDepthFunc(GL_GREATER);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	*foo = window;

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    InputWrapper* wrapper = static_cast<InputWrapper*>(glfwGetWindowUserPointer(window));
    wrapper->MouseCallback(window, xPos, yPos);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    InputWrapper* wrapper = static_cast<InputWrapper*>(glfwGetWindowUserPointer(window));
    wrapper->ScrollCallback(window, xOffset, yOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputWrapper* wrapper = static_cast<InputWrapper*>(glfwGetWindowUserPointer(window));
    wrapper->KeyCallback(window, key, scancode, action, mods);
}

void timer_message(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end, std::string message)
{
    double milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << message << milliseconds << std::endl;
}

#pragma message WARN("REFACTOR THIS INTO LOADING A SCENE FROM DISK")
Scene load_scene()
{
    SceneGraph graph = SceneGraph();

    // Camera initialization
    Camera cam = Camera();
    cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));
    std::vector<Camera> cams = std::vector<Camera>();
    cams.push_back(cam);
    
    std::vector<Shader> shaders = std::vector<Shader>();

	Cubemap skybox = Cubemap();

    Scene scene = Scene(graph, cams, ModelLoader("Resources/"), shaders, skybox);

    return scene;
}

Scene load_normal_scene()
{
    SceneGraph graph = SceneGraph(1);
    
    // Camera initialization
    Camera cam = Camera();
    cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));
    std::vector<Camera> cams = std::vector<Camera>();
    cams.push_back(cam);
    
//    std::vector<Model> models = std::vector<Model>();
//    models.push_back(create_brick_wall());
    
    std::vector<Shader> shaders = std::vector<Shader>();
    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/NormalMaps.vert").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/NormalMaps.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());
    shaders.push_back(standard_shader);
    
    while (shaders.size() < 9)
    {
        shaders.push_back(Shader());
    }
    
    Cubemap skybox = Cubemap();
    
    Scene scene = Scene(graph, cams, ModelLoader("Resources/"), shaders, skybox);
    
    return scene;
}
