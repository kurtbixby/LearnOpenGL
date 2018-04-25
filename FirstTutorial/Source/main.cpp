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
#include "Headers/Framebuffer.h"
#include "Headers/InputWrapper.h"
#include "Headers/Model.h"
#include "Headers/PrimitivesLoader.h"
#include "Headers/RenderConfig.h"
#include "Headers/Renderer.h"
#include "Headers/Scene.h"
#include "Headers/SceneGraph.h"
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

#define WIDTH 800
#define HEIGHT 600

#define SHADOW_RES 2048

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
    RenderConfig config = RenderConfig();
    
    // Create InputWrapper?
    InputWrapper inputWrapper = InputWrapper();
	GLFWwindow* window;
	if (create_window(&window, inputWrapper, config.WindowWidth(), config.WindowHeight()))
	{
		std::cerr << "Error creating window" << std::endl;
		return -1;
	}
    
    Renderer renderer = Renderer(config);
    
    Scene scene = load_scene();
//    Scene scene = load_normal_scene();
    
//    if (config.ShadowmapsEnabled())
//    {
//        // Switch this to use SceneRenderer
//        Framebuffer shadow_map_buffer = Framebuffer(SHADOW_RES, SHADOW_RES, 1, false);
//        scene.GenerateShadowMaps(shadow_map_buffer);
//    }
    
    SceneRenderer sceneRenderer = SceneRenderer(&scene, SHADOW_RES);
    
    Timer frame_timer = Timer();
    // main loop
    while (!glfwWindowShouldClose(window))
    {
        frame_timer.Start();
        // process any input
        Input input = inputWrapper.TakeInput(window);
        scene.TakeInput(input);
        
        scene.Simulate();
        
        renderer.RenderScene(sceneRenderer);
        
        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
        // break;

		glBindTexture(GL_TEXTURE_2D, 0);
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

#warning REFACTOR THIS INTO LOADING A SCENE FROM DISK
Scene load_scene()
{
    SceneGraph graph = SceneGraph();

    // Camera initialization
    Camera cam = Camera();
    cam.SetAspectRatio(WIDTH / static_cast<float>(HEIGHT));
    std::vector<Camera> cams = std::vector<Camera>();
    cams.push_back(cam);

    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTexturesInstanced_ShadMap_ManyLights.vert").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection_ShadMapSmooth_ManyLights_Bloom.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());

	boost::filesystem::path transparent_fragment_shader_path = boost::filesystem::path("Shaders/Transparency_Blended.frag").make_preferred();
	Shader transparent_shader = Shader(vertex_shader_path.string().c_str(), transparent_fragment_shader_path.string().c_str());

    boost::filesystem::path outline_fragment_shader_path = boost::filesystem::path("Shaders/Outline.frag").make_preferred();
    Shader outline_shader = Shader(vertex_shader_path.string().c_str(), outline_fragment_shader_path.string().c_str());

	boost::filesystem::path skybox_vertex_shader_path = boost::filesystem::path("Shaders/SkyboxBuff.vert").make_preferred();
	boost::filesystem::path skybox_fragment_shader_path = boost::filesystem::path("Shaders/Skybox.frag").make_preferred();
	Shader skybox_shader = Shader(skybox_vertex_shader_path.string().c_str(), skybox_fragment_shader_path.string().c_str());
    
    boost::filesystem::path bonus_vertex_shader_path = boost::filesystem::path("Shaders/MultipleTextures.vert").make_preferred();
    boost::filesystem::path bonus_geometry_shader_path = boost::filesystem::path("Shaders/Normals.geom").make_preferred();
    boost::filesystem::path bonus_fragment_shader_path = boost::filesystem::path("Shaders/Old/First.frag").make_preferred();
    Shader bonus_shader = Shader(bonus_vertex_shader_path.string().c_str(), bonus_geometry_shader_path.string().c_str(), bonus_fragment_shader_path.string().c_str());
    
    boost::filesystem::path alt_light_vertex_shader_path = boost::filesystem::path("Shaders/MultipleTexturesInstanced.vert").make_preferred();
    boost::filesystem::path alt_light_fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection_Blinn.frag").make_preferred();
    Shader alt_light_shader = Shader(alt_light_vertex_shader_path.string().c_str(), alt_light_fragment_shader_path.string().c_str());

    boost::filesystem::path shadow_map_vertex_shader_path = boost::filesystem::path("Shaders/Dir_ShadowsInstanced.vert").make_preferred();
    boost::filesystem::path shadow_map_fragment_shader_path = boost::filesystem::path("Shaders/Dir_Shadows.frag").make_preferred();
    Shader shadow_map_shader = Shader(shadow_map_vertex_shader_path.string().c_str(), shadow_map_fragment_shader_path.string().c_str());
    
    boost::filesystem::path pnt_shadow_map_vert_shader_path = boost::filesystem::path("Shaders/Point_Shadows.vert").make_preferred();
    boost::filesystem::path pnt_shadow_map_geom_shader_path = boost::filesystem::path("Shaders/Point_Shadows.geom").make_preferred();
    boost::filesystem::path pnt_shadow_map_frag_shader_path = boost::filesystem::path("Shaders/Point_Shadows.frag").make_preferred();
    Shader pnt_shadow_map_shader = Shader(pnt_shadow_map_vert_shader_path.string().c_str(), pnt_shadow_map_geom_shader_path.string().c_str(), pnt_shadow_map_frag_shader_path.string().c_str());
    
    boost::filesystem::path spt_shadow_map_vert_shader_path = boost::filesystem::path("Shaders/Spot_Shadows.vert").make_preferred();
    boost::filesystem::path spt_shadow_map_frag_shader_path = boost::filesystem::path("Shaders/Spot_Shadows.frag").make_preferred();
    Shader spt_shadow_map_shader = Shader(spt_shadow_map_vert_shader_path.string().c_str(), spt_shadow_map_frag_shader_path.string().c_str());
    
    boost::filesystem::path lights_vert_shader_path = boost::filesystem::path("Shaders/Lights.vert").make_preferred();
    boost::filesystem::path lights_frag_shader_path = boost::filesystem::path("Shaders/Lights_Bloom.frag").make_preferred();
    Shader lights_shader = Shader(lights_vert_shader_path.string().c_str(), lights_frag_shader_path.string().c_str());
    
    std::vector<Shader> shaders = std::vector<Shader>();
    shaders.push_back(standard_shader);
	shaders.push_back(transparent_shader);
    shaders.push_back(outline_shader);
	shaders.push_back(skybox_shader);
    shaders.push_back(bonus_shader);
    shaders.push_back(alt_light_shader);
    shaders.push_back(shadow_map_shader);
    shaders.push_back(pnt_shadow_map_shader);
    shaders.push_back(spt_shadow_map_shader);
    shaders.push_back(lights_shader);

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
