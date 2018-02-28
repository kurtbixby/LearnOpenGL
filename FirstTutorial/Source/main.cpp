#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <chrono>

#define _USE_MATH_DEFINES
#include <cmath>

//#include "Headers/Debug.h"

#include "Headers/Camera.h"
#include "Headers/Framebuffer.h"
#include "Headers/InputWrapper.h"
#include "Headers/Model.h"
#include "Headers/Primitives.h"
#include "Headers/Scene.h"
#include "Headers/SceneGraph.h"
#include "Headers/Shader.h"
#include "Headers/Structs.h"

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Headers/ModelLoader.h"

#define WIDTH 800
#define HEIGHT 600

int create_window(GLFWwindow** foo, InputWrapper& inputWrapper);
Input get_input(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

void generate_cube_locations(const unsigned number, glm::vec3* const cube_array);
Scene load_scene();

int main()
{
    // Get configuration?
    // Create InputWrapper?
    InputWrapper inputWrapper = InputWrapper();
	GLFWwindow* window;
	if (create_window(&window, inputWrapper))
	{
		std::cerr << "Error creating window" << std::endl;
		return -1;
	}

	// Create standard shader
	boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/Screen.vert").make_preferred();
	boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/Kernel.frag").make_preferred();
	Shader screenShader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());

	// Create intermediary framebuffer
	Framebuffer output_fb = Framebuffer();
	output_fb.AddTextureAttachment(FBAttachment::Color);
	output_fb.AddRenderbufferAttachment(FBAttachment::DepthStencil);
    
    Framebuffer multi_sample_fb = Framebuffer();
    multi_sample_fb.AddTextureAttachment(FBAttachment::Color, 4);
    multi_sample_fb.AddRenderbufferAttachment(FBAttachment::DepthStencil, 4);

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
//    float kernel[9] = {
//        1, 1, 1,
//        1, -8, 1,
//        1, 1, 1
//    };

	// Identity
    float kernel[9] = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    };
    
    Scene scene = load_scene();

//    uint32_t frame_number = 0;
    uint32_t frames_rendered = 0;
    std::chrono::steady_clock::time_point previous_time = std::chrono::steady_clock::now();
    
    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // process any input
        Input input = inputWrapper.TakeInput(window);
        scene.TakeInput(input);

		// Enable texture framebuffer
		multi_sample_fb.Use();

		// (Re)enable depth for main scene
		glEnable(GL_DEPTH_TEST);
        scene.Render();
		// Disable depth for screen quad
		glDisable(GL_DEPTH_TEST);
        
        multi_sample_fb.DownsampleToFramebuffer(output_fb);
        uint32_t frame_buffer_target = output_fb.RetrieveColorBuffer(0).TargetName;
        
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
        
//        debug::openGL_Errors();
        
		// bind fb texture
		glBindTexture(GL_TEXTURE_2D, frame_buffer_target);
		// send kernel to shader
		screenShader.SetFloats("kernel", 9, &kernel[0]);
		// draw screen quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

        // buffer swap and poll for new events
        glfwSwapBuffers(window);
        glfwPollEvents();
        // break;

		glBindTexture(GL_TEXTURE_2D, 0);
//        frame_number++;
        frames_rendered++;
        
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        double milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - previous_time).count();
        if (milliseconds > 1000)
        {
            float frame_time = milliseconds / frames_rendered;
            std::cout << "Avg frame time: " << frame_time << std::endl;
            previous_time = current_time;
            frames_rendered = 0;
        }
    }

    glfwTerminate();
    return 0;
}

int create_window(GLFWwindow** foo, InputWrapper& inputWrapper)
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

	// glad load all opengl fxn pointers
	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
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
    models.push_back(crysis);
	models.push_back(create_box());
//    models.push_back(create_plane());
//    models.push_back(create_quad());

    boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTexturesInstanced.vert").make_preferred();
//    boost::filesystem::path geometry_shader_path = boost::filesystem::path("Shaders/Identity.geom").make_preferred();
    boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection.frag").make_preferred();
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
    boost::filesystem::path bonus_fragment_shader_path = boost::filesystem::path("Shaders/First.frag").make_preferred();
    Shader bonus_shader = Shader(bonus_vertex_shader_path.string().c_str(), bonus_geometry_shader_path.string().c_str(), bonus_fragment_shader_path.string().c_str());
    
    boost::filesystem::path alt_light_vertex_shader_path = boost::filesystem::path("Shaders/MultipleTexturesInstanced.vert").make_preferred();
    boost::filesystem::path alt_light_fragment_shader_path = boost::filesystem::path("Shaders/TexturesReflection_Blinn.frag").make_preferred();
    Shader alt_light_shader = Shader(alt_light_vertex_shader_path.string().c_str(), alt_light_fragment_shader_path.string().c_str());

    std::vector<Shader> shaders = std::vector<Shader>();
    shaders.push_back(standard_shader);
	shaders.push_back(transparent_shader);
    shaders.push_back(outline_shader);
	shaders.push_back(skybox_shader);
    shaders.push_back(bonus_shader);
    shaders.push_back(alt_light_shader);

	Cubemap skybox = Cubemap();

    Scene scene = Scene(graph, cams, models, shaders, skybox);

    return scene;
}
