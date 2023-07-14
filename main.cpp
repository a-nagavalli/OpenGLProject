#include <glad/glad.h>
#include <include/GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <cmath>

#include "ShaderProgram.h"
#include "model.h"
#include "Object.h"
#include "Skybox.h"

enum CameraType {
	FIRST_PERSON,
	FLYING
};

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float MOVEMENT_SPEED = 4.0f;
const float GROUND_Y = 1.6f;
const float PLAYER_HEIGHT = 1.4f;
const float X_BOUND_LEFT = -2.5f;
const float X_BOUND_RIGHT = 18.5f;
const float Z_BOUND_LEFT = -6.5f;
const float Z_BOUND_RIGHT = 14.5f;
const CameraType camType = FIRST_PERSON;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void initLight(const ShaderProgram& program);
void enforceBounds(glm::vec3& position);

// camera information
/*
	If you want to change the camera's initial orientation, the cameraFront direction and the
	yaw must align to make it smooth on startup (both pointing in the z-direction is a good choice:

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	float yaw = 90.0f;

	They will only align perfectly at values of x and y such that sin(y) / cos(x) = 1.0
	(yaw increments of 45 degrees)
*/
glm::vec3 cameraPos = glm::vec3(18.0f, GROUND_Y + PLAYER_HEIGHT, 13.0f);
glm::vec3 cameraFront = glm::vec3(float(-sqrt(2)), 0.0f, float(-sqrt(2)));
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -135.0f;
float pitch = 0.0f;

// used for determining mouse movement offsets (for the camera)
float lastX = WINDOW_WIDTH / 2.0;
float lastY = WINDOW_HEIGHT / 2.0;
bool firstMouse = true;

// used for calculating movement speed
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// where the sunlight is coming from
const glm::vec3 sunlightPos(-50.0f, 100.0f, 50.0f);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGLProject", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// set callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// must initialize GLAD before using gl functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// create model shader program
	ShaderFile vertexShaderFile("VertexShader.vert", "vertex");
	ShaderFile fragmentShaderFile("FragmentShader.frag", "fragment");
	ShaderProgram shaderProgram(vertexShaderFile, fragmentShaderFile);

	// create lightbulb shader program
	/*vertexShaderFile = ShaderFile("LightbulbVertexShader.vert", "vertex");
	fragmentShaderFile = ShaderFile("LightbulbFragmentShader.frag", "fragment");
	ShaderProgram lightbulbShaderProgram(vertexShaderFile, fragmentShaderFile);*/

	// create skybox shader program
	vertexShaderFile = ShaderFile("SkyboxVertexShader.vert", "vertex");
	fragmentShaderFile = ShaderFile("SkyboxFragmentShader.frag", "fragment");
	ShaderProgram skyboxShaderProgram(vertexShaderFile, fragmentShaderFile);

	// set textures to load in the correct orientation
	// NOTE: some textures may still be upside-down; if you're seeing black where there should be a texture, try flipping the texture image itself upside-down
	// stbi_set_flip_vertically_on_load(true);

	Skybox skybox(".bmp", "Skybox Textures", skyboxShaderProgram);
	Object house("Textured Models/House2/House2.obj");
	Object grass("Textured Models/grassground/grassground.obj");
	Object tree1("Textured Models/Tree/Tree.obj");
	Object tree2("Textured Models/Tree/Tree.obj");
	//Object lightbulb1("Textured Models/Lightbulb/Lightbulb.obj");

	//grass.Translate(0.0f, GROUND_Y, 0.0f);
	grass.Translate(0.0f, GROUND_Y, -4.0f);
	grass.Scale(4.0f, 1.0f, 4.0f);

	house.Translate(2.0f, 0.0f, 3.0f);

	tree1.Translate(0.0f, GROUND_Y, 4.0f);
	tree2.Translate(13.0f, GROUND_Y, -1.0f);
	//lightbulb1.Translate(sunlightPos.x, sunlightPos.y, sunlightPos.z);

	// create projection matrix for the models (doesn't need to be updated every frame)
	shaderProgram.use();
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), float(1.0 * WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 100.0f);
	shaderProgram.setUniformMatrix("projection", projection);

	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();		// frame delta should be calculated before everything else
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClear(GL_DEPTH_BUFFER_BIT);

		shaderProgram.use();

		// create view matrix for the models
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));
		shaderProgram.setUniformMatrix("view", view);

		// set light properties
		initLight(shaderProgram);

		// set view and projection matrices for lightbulb
		//lightbulbShaderProgram.use();
		//lightbulbShaderProgram.setUniformMatrix("view", view);
		//lightbulbShaderProgram.setUniformMatrix("projection", projection);

		house.Draw(shaderProgram);
		grass.Draw(shaderProgram);
		tree1.Draw(shaderProgram);
		tree2.Draw(shaderProgram);
		//lightbulb1.Draw(lightbulbShaderProgram);
		skybox.Draw(view, projection);		// skybox drawn last

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)	// called every time window is resized
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = MOVEMENT_SPEED * deltaTime;
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));	// the cross product of the front and up vectors will be the right vector (orthogonal to both the other vectors)
	glm::vec3 forward = glm::normalize(glm::cross(cameraRight, cameraUp));	// the cross product of the right and up vectors will be a vector facing in the forward direction (relative to camera)

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {		
		switch (camType) {
			case FIRST_PERSON:
				cameraPos -= cameraSpeed * forward;
				enforceBounds(cameraPos);
				break;

			case FLYING:
				cameraPos += cameraSpeed * cameraFront;
				break;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		switch (camType) {
			case FIRST_PERSON:
				cameraPos += cameraSpeed * forward;
				enforceBounds(cameraPos);
				break;

			case FLYING:
				cameraPos -= cameraSpeed * cameraFront;
				break;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		switch (camType) {
			case FIRST_PERSON:
				cameraPos -= cameraSpeed * cameraRight;
				enforceBounds(cameraPos);
				break;

			case FLYING:
				cameraPos -= cameraSpeed * cameraRight;
				break;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		switch (camType) {
			case FIRST_PERSON:
				cameraPos += cameraSpeed * cameraRight;
				enforceBounds(cameraPos);
				break;

			case FLYING:
				cameraPos += cameraSpeed * cameraRight;
				break;
		}
	}

	/*if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		std::cout << "(" << int(cameraPos.x) << ".0f, GROUND_Y, " << int(cameraPos.z) << ".0f)" << std::endl;*/
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront.y = sin(glm::radians(pitch));
	cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
}

void initLight(const ShaderProgram& program)
{
	program.setVec3("sunlight.position", sunlightPos);
	program.setVec3("sunlight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	program.setFloat("sunlight.ambientIntensity", 0.2f);
	program.setVec3("sunlight.ambientColor", glm::vec3(1.0f, 1.0f, 1.0f));
	program.setVec3("sunlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	program.setInt("sunlight.shininess", 4);
	program.setVec3("viewPos", cameraPos);
}

void enforceBounds(glm::vec3& position)
{
	if (position.x < X_BOUND_LEFT)
		position.x = X_BOUND_LEFT;
	if (position.x > X_BOUND_RIGHT)
		position.x = X_BOUND_RIGHT;
	if (position.z < Z_BOUND_LEFT)
		position.z = Z_BOUND_LEFT;
	if (position.z > Z_BOUND_RIGHT)
		position.z = Z_BOUND_RIGHT;
}