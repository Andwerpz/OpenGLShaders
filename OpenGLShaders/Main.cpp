#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "Model.h"
#include "Shader.h"

#include <iostream>
#include <filesystem>
#include <string>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#endif

glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(-0.7f,  -0.2f,  -2.0f),
};

//light settings
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

//--camera controls--
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = 0;	float pitch = 0;
float fov = 75;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float lastX = 400, lastY = 300;

bool firstMouse = true;	//just so that the camera doesn't jerk around when first entering the window

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 100.0f)
		fov = 100.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) {
		firstMouse = false;
		lastX = xpos;
		lastY = ypos;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
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

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//--setting up shaders--
	Shader ourShader("Shader.vert", "Shader.frag");
	Shader lightShader("Shader.vert", "ShaderLight.frag");	//to draw the light sources
	Shader singleColorShader("Shader.vert", "ShaderSingleColor.frag");	//for outlining

	//--loading models--
	std::filesystem::path path = std::filesystem::current_path();
	std::string path_string(path.string());
	stbi_set_flip_vertically_on_load(true);	//flip textures vertically before loading model
	Model backpack(path_string + "\\res\\backpack\\backpack.obj");
	Model light_bulb(path_string + "\\res\\light_bulb\\light_bulb.obj");
	

	//WIREFRAME MODE:
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//--glfw toggle settings--
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//available vertex attributes
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
		<< std::endl;

	int counter = 0;
	int frames = 0;
	double curSecond = 0;
	float outlineThickness = 0.1f;

	while (!glfwWindowShouldClose(window))
	{
		//--frame counter--
		counter++;
		if (glfwGetTime() - 1 > curSecond) {
			frames = counter;
			counter = 0;
			curSecond++;
		}

		glfwSetWindowTitle(window, (std::string("FPS : ") + std::to_string(frames)).c_str());
		
		//--player controls--
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		//--dealing with buffers--
		//clearing buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	//rgba
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//dealing with stencil buffer
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer

		//--setting up transformation matrices--
		//we have to do these every frame, since these might change
		glm::mat4 world = glm::mat4(1.0f);
		world = glm::rotate(world, (float) 0, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float) 0, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), 1920.0f / 1080.0f, 0.1f, 100.0f);

		//--rendering --
		
		//initializing object shader
		ourShader.use();	

		ourShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		ourShader.setVec3("viewPos", cameraPos);
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		ourShader.setFloat("material.shininess", 32.0);

		//point light
		ourShader.setVec3( "pointLights[0].position", pointLightPositions[0]);
		ourShader.setVec3( "pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		ourShader.setVec3( "pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		ourShader.setVec3( "pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("pointLights[0].constant", 1.0f);
		ourShader.setFloat("pointLights[0].linear", 0.09);
		ourShader.setFloat("pointLights[0].quadratic", 0.032);

		ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("pointLights[1].constant", 1.0f);
		ourShader.setFloat("pointLights[1].linear", 0.09);
		ourShader.setFloat("pointLights[1].quadratic", 0.032);

		//flashlight
		ourShader.setVec3("spotLight.position", cameraPos);
		ourShader.setVec3("spotLight.direction", cameraFront);
		ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("spotLight.constant", 1.0f);
		ourShader.setFloat("spotLight.linear", 0.09);
		ourShader.setFloat("spotLight.quadratic", 0.032);
		ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(0.0f)));
		ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(0.0f)));	

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);	//set the stencil color to 1
		glStencilMask(0xFF);
		backpack.Draw(ourShader);

		//drawing outline
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);	//setting so that it will only draw where the previous backpack hasn't been drawn
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		singleColorShader.use();

		singleColorShader.setVec3("viewPos", cameraPos);
		singleColorShader.setMat4("view", view);
		singleColorShader.setMat4("projection", projection);
		singleColorShader.setMat4("model", model);

		singleColorShader.setFloat("scale", outlineThickness);

		backpack.Draw(singleColorShader);

		glStencilMask(0xFF);	//resetting stencil settings for future draw commands
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_DEPTH_TEST);
		
		//drawing light sources

		lightShader.use();
		lightShader.setVec3("lightColor", lightColor);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);

		for (int i = 0; i < 2; i++) {
			model = glm::mat4(1.0);
			model = glm::translate(world, pointLightPositions[i]);
			
			lightShader.setMat4("model", model);

			light_bulb.Draw(lightShader);
		}

		//--swap buffers--
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}



