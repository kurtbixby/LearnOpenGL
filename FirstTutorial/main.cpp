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
#include "Structs.h"
#include "Model.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
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
// unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type = GL_REPEAT);

Mesh create_box();
Mesh create_plane();
std::pair<int, glm::vec3> create_object(const int mesh, const glm::vec3& transform);

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

unordered_map<string, Model> loaded_models_ = unordered_map<string, Model>();

int main()
{
	GLFWwindow* window;
	if (create_window(&window))
	{
		std::cout << "Error creating window" << std::endl;
		return -1;
	}

	boost::filesystem::path vertex_shader_path = boost::filesystem::path("Shaders/MultipleTextures.vert").make_preferred();
	boost::filesystem::path fragment_shader_path = boost::filesystem::path("Shaders/MultipleTextures.frag").make_preferred();
    Shader standard_shader = Shader(vertex_shader_path.string().c_str(), fragment_shader_path.string().c_str());
    boost::filesystem::path outline_fragment_shader_path = boost::filesystem::path("Shaders/Outline.frag").make_preferred();
    Shader outline_shader = Shader(vertex_shader_path.string().c_str(), outline_fragment_shader_path.string().c_str());

    // vertex data
 //    Vertex vertices[] = {
 //        create_vertex( -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f ),
 //        create_vertex(  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f ),
 //        create_vertex(  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f ),
 //        create_vertex(  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f ),
 //        create_vertex( -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f ),
 //        create_vertex( -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f ),

 //        create_vertex( -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f ),
 //        create_vertex(  0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f ),
 //        create_vertex(  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f ),
 //        create_vertex(  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f ),
 //        create_vertex( -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f ),
 //        create_vertex( -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f ),

 //        create_vertex( -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex( -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f ),
 //        create_vertex( -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex( -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex( -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f ),
 //        create_vertex( -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f ),

 //        create_vertex(  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex(  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f ),
 //        create_vertex(  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex(  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex(  0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f ),
 //        create_vertex(  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f ),

 //        create_vertex( -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex(  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f ),
 //        create_vertex(  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex(  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex( -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f ),
 //        create_vertex( -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f ),

 //        create_vertex( -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f ),
 //        create_vertex(  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f ),
 //        create_vertex(  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex(  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f ),
 //        create_vertex( -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f ),
 //        create_vertex( -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f ),
 //    };

 //    // create a Vertex Buffer Object to hold the vertices
 //    // the ID of the VBO is 1
 //    unsigned int vbo;
 //    glGenBuffers(1, &vbo);

	// // Create Vertex Array and related buffers
	// unsigned int vao;
	// glGenVertexArrays(1, &vao);
 //    glBindVertexArray(vao);
 //    glBindBuffer(GL_ARRAY_BUFFER, vbo); // sets VBO to the current GL buffer array
 //    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // sends data from to the buffer
	
	// // Position
 //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Position)));
 //    glEnableVertexAttribArray(0);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Normal)));
	// glEnableVertexAttribArray(1);
 //    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
 //    glEnableVertexAttribArray(2);

	// unsigned int lightVao;
	// glGenVertexArrays(1, &lightVao);
	// glBindVertexArray(lightVao);
	// glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Position)));
	// glEnableVertexAttribArray(0);

	// // Texture Loading
	// boost::filesystem::path diffuse_texture_path = boost::filesystem::path("resources/container2_diffuse.png").make_preferred();
	// unsigned int diffuse_texture = load_texture(diffuse_texture_path.string().c_str(), GL_RGBA);
 //    boost::filesystem::path specular_texture_path = boost::filesystem::path("resources/container2_specular.png").make_preferred();
 //    unsigned int specular_texture = load_texture(specular_texture_path.string().c_str(), GL_RGBA);

	// // Cube generation
	// const unsigned int cubes = 10;
	// glm::vec3 cube_locations[cubes];
	// generate_cube_locations(cubes, cube_locations);

	//Model::Init();

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

    Light lights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
	
    // main loop
    while (!glfwWindowShouldClose(window))
    {
        int DIR_LIGHTS = 0;
        int POINT_LIGHTS = 0;
        int SPOT_LIGHTS = 0;

        // process any input
        process_input(window, cam);

		glm::vec3 camPosition = cam.GetPosition();
        glm::vec3 camDirection = cam.GetDirection();

        // render section of main loop
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // state setter
        // glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state user

		glm::mat4 projection = cam.GetProjection();
		glm::mat4 view = cam.MakeViewMat();
        // Uncomment to have the light orbit
		// lightDirection = lightMultiplication * lightDirection;

		glm::vec3 redLight(1.0f, 0.0f, 0.0f);
        glm::vec3 greenLight(0.0f, 1.0f, 0.0f);
        glm::vec3 blueLight(1.0f, 1.0f, 1.0f);

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

        standard_shader.Use();
        standard_shader.SetMatrix4fv("projection", glm::value_ptr(projection));
        standard_shader.SetMatrix4fv("view", glm::value_ptr(view));
		//standard_shader.SetVec3("viewPos", camPosition.x, camPosition.y, camPosition.z);
        standard_shader.SetInt("DIR_LIGHTS", std::min(MAX_DIR_LIGHTS, DIR_LIGHTS));
        standard_shader.SetInt("POINT_LIGHTS", std::min(MAX_POINT_LIGHTS, POINT_LIGHTS));
        standard_shader.SetInt("SPOT_LIGHTS", std::min(MAX_SPOT_LIGHTS, SPOT_LIGHTS));

        for (int i = 0; i < DIR_LIGHTS; i++)
        {
            Light* light = &lights[i];
            send_direction_light(standard_shader, *light, i);
        }

        for (int i = 0; i < POINT_LIGHTS; i++)
        {
            PointLight* light = &pointLights[i];
            send_point_light(standard_shader, *light, i);
        }

        for (int i = 0; i < SPOT_LIGHTS; i++)
        {
            SpotLight* light = &spotLights[i];
            send_spot_light(standard_shader, *light, i);
        }

        std::vector<Mesh> meshes = std::vector<Mesh>();
        meshes.push_back(create_box());
        meshes.push_back(create_plane());

        std::vector<Object> objects = std::vector<Object>();
        objects.push_back(Object(glm::vec3(0.0f), 1, 1.0f, false));
        objects.push_back(Object(glm::vec3(-1.0f, 0.0f, -1.0f), 0, 1.0f, true));
        objects.push_back(Object(glm::vec3(2.0f, 0.0f, 0.0f), 0, 1.0f, true));

        std::vector<Object> stenciled = std::vector<Object>();

        for(int i = 0; i < objects.size(); i++)
        {
            Object& object = objects[i];
            if (object.Outlined_)
            {
                stenciled.push_back(object);
                continue;
            }
            else
            {
                Mesh mesh = meshes[object.Mesh_];
                glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
                model = glm::scale(model, object.Scale_);
                standard_shader.SetMatrix4fv("model", glm::value_ptr(model));
                mesh.Draw(standard_shader);
            }
        }

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        // First pass
        for (int i = 0; i < stenciled.size(); i++)
        {
            Object& object = stenciled[i];

            Mesh mesh = meshes[object.Mesh_];
            glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
            model = glm::scale(model, object.Scale_);
            standard_shader.SetMatrix4fv("model", glm::value_ptr(model));
            mesh.Draw(standard_shader);
        }

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        // Outline pass
        outline_shader.Use();
        outline_shader.SetMatrix4fv("projection", glm::value_ptr(projection));
        outline_shader.SetMatrix4fv("view", glm::value_ptr(view));
        for (int i = 0; i < stenciled.size(); i++)
        {
            Object& object = stenciled[i];

            Mesh mesh = meshes[object.Mesh_];
            glm::mat4 model = glm::translate(glm::mat4(1.0f), object.Transform_);
            model = glm::scale(model, object.Scale_ * 1.1f);
            outline_shader.SetMatrix4fv("model", glm::value_ptr(model));
            mesh.Draw(outline_shader);
        }
        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);

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
    // glDepthFunc(GL_GREATER);
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

Mesh create_box()
{
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

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

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
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
         5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f,

         5.0f, -0.5f,  5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f,
         5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f                                
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

std::pair<int, glm::vec3> create_object(const int mesh, const glm::vec3& transform)
{
    return std::pair<int, glm::vec3>(mesh, transform);
}
