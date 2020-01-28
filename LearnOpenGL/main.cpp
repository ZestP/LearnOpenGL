#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ctime>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"



#include "model.h"
#define TEXTURE_WIDTH 1600
#define TEXTURE_HEIGHT 800
using namespace std;

const GLuint SHADOW_WIDTH = 8196, SHADOW_HEIGHT = 8196;
GLuint depthMapFBO;
GLuint depthMap;


float screenWidth = 1600, screenHeight = 800;
float lastX = 800, lastY = 400;
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

glm::vec3 cubePositions[] = {
  glm::vec3(0.0f,  0.0f,  0.0f),
  glm::vec3(2.0f,  5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3(2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3(1.3f, -2.0f, -2.5f),
  glm::vec3(1.5f,  2.0f, -2.5f),
  glm::vec3(1.5f,  0.2f, -1.5f),
  glm::vec3(-1.3f,  1.0f, -1.5f)
};

unsigned int VBO;
unsigned int vertexShader;
unsigned int VAO;
unsigned int lightVAO;
unsigned int EBO;
unsigned int fragmentShader;
unsigned int shaderProgram;
unsigned int framebuffer,fbo2,renderbuffer, rendertarget,rin;
Shader* ourShader, *diffuseShader,*lampShader,*blinnPhongShader,*simpleDepthShader;
int cnt = 0;
unsigned int texture,texture2;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw, pitch,fov=45.0f;
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

glm::vec3 lightPos(-6.0f, 10.0f, 3.0f),lightDir(0.6f, -1.0f, -0.3f);

Model* testModel;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
Shader *debugDepthQuad;
unsigned int quadVAO = 0;
unsigned int quadVBO;
GLfloat near_plane = 1.0f, far_plane = 7000.5f;

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void RenderScene()
{
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
//glBindTexture(GL_TEXTURE_2D, rendertarget);
	
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(fov), screenWidth / screenHeight, 0.1f, 100.0f);
	glBindVertexArray(VAO);


	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);

	// render container
	ourShader->use();
	int modelLoc = glGetUniformLocation(ourShader->ID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	int viewLoc = glGetUniformLocation(ourShader->ID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	int projLoc = glGetUniformLocation(ourShader->ID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (unsigned int i = 0; i < 10; i++)
	{
		glm::mat4 model=glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle+ (float)glfwGetTime()*1000.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		ourShader->setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	lampShader->use();
	model = glm::mat4(1.0f);
	//model = glm::translate(model, lightPos);
	//model = glm::scale(model, glm::vec3(0.2f));
	lampShader->setMat4("projection", projection);
	lampShader->setMat4("view", view);
	lampShader->setMat4("model", model);
	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	blinnPhongShader->use();
	blinnPhongShader->setMat4("projection", projection);
	blinnPhongShader->setMat4("view", view);

	// render the loaded model
	glm::mat4 model2 = glm::mat4(1.0f);
	model2 = glm::translate(model2, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model2 = glm::rotate(model2, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model2 = glm::scale(model2, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	blinnPhongShader->setMat4("model", model2);
	blinnPhongShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
	blinnPhongShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	blinnPhongShader->setVec3("lightPos", lightPos);
	blinnPhongShader->setVec3("viewPos", cameraPos);
	blinnPhongShader->setVec3("mat.ambient", 1.0f, 0.5f, 0.31f);
	blinnPhongShader->setVec3("mat.diffuse", 1.0f, 0.5f, 0.31f);
	blinnPhongShader->setVec3("mat.specular", 0.5f, 0.5f, 0.5f);
	blinnPhongShader->setFloat("mat.shininess", 32.0f);

	glm::vec3 lightColor;
	lightColor.x = sin(glfwGetTime() * 2.0f);
	lightColor.y = sin(glfwGetTime() * 0.7f);
	lightColor.z = sin(glfwGetTime() * 1.3f);

	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // 降低影响
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // 很低的影响

	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	blinnPhongShader->setMat4("lightSpaceMatrixLocation", lightSpaceMatrix);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	blinnPhongShader->setInt("shadowMap", 5);

	blinnPhongShader->setVec3("dirLight.direction",lightDir);

	blinnPhongShader->setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
	blinnPhongShader->setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f); // 将光照调暗了一些以搭配场景
	blinnPhongShader->setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
	blinnPhongShader->setFloat("dirLight.constant", 1.0f);
	blinnPhongShader->setFloat("dirLight.linear", 0.09f);
	blinnPhongShader->setFloat("dirLight.quadratic", 0.032f);

	blinnPhongShader->setVec3("spotLight.position", cameraPos);
	blinnPhongShader->setVec3("spotLight.direction", cameraFront);
	blinnPhongShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	blinnPhongShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
	blinnPhongShader->setVec3("spotLight.ambient", 0.2f, 0.2f, 0.2f);
	blinnPhongShader->setVec3("spotLight.diffuse", 0.5f, 0.5f, 0.5f); // 将光照调暗了一些以搭配场景
	blinnPhongShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	blinnPhongShader->setFloat("spotLight.constant", 1.0f);
	blinnPhongShader->setFloat("spotLight.linear", 0.09f);
	blinnPhongShader->setFloat("spotLight.quadratic", 0.032f);
	testModel->Draw(*blinnPhongShader);
	//
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glBlitFramebuffer(0, 0, 1599, 799, 0, 0, 1599, 799, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlitFramebuffer(0, 0, 1599, 799, 0, 0, 1599, 799, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

}

void RenderScene(Shader* shader)
{
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
//glBindTexture(GL_TEXTURE_2D, rendertarget);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
	glBindVertexArray(VAO);

	for (unsigned int i = 0; i < 10; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle + (float)glfwGetTime() * 1000.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		shader->setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}



	//// bind textures on corresponding texture units

	//// render container

	//int modelLoc = glGetUniformLocation(shader->ID, "model");
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//
	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	

	model = glm::mat4(1.0f);
	//model = glm::translate(model, lightPos);
	//model = glm::scale(model, glm::vec3(0.2f));
	
	shader->setMat4("model", model);
	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	
	// render the loaded model
	glm::mat4 model2 = glm::mat4(1.0f);
	model2 = glm::translate(model2, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model2 = glm::rotate(model2, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model2 = glm::scale(model2, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	shader->setMat4("model", model2);
	
	testModel->Draw(*shader);
	
}

void ConfigureShaderAndMatrices()
{
	
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	simpleDepthShader->use();
	simpleDepthShader->setMat4("lightSpaceMatrixLocation", lightSpaceMatrix);
	
}

void render(GLFWwindow* window)
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;


	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices();

	// 1. 首选渲染深度贴图
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	RenderScene(simpleDepthShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 2. 像往常一样渲染场景，但这次使用深度贴图
	glViewport(0, 0, screenWidth, screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//// render Depth map to quad for visual debugging
	//		// ---------------------------------------------
	//debugDepthQuad->use();
	//debugDepthQuad->setFloat("near_plane", near_plane);
	//debugDepthQuad->setFloat("far_plane", far_plane);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//renderQuad();


	RenderScene();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // 这个bool变量初始时是设定为true的
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFront = glm::normalize(front);
}

void init(GLFWwindow* window)
{
	srand((unsigned int)time(NULL));
	//// 创建纹理
	//glGenTextures(1, &rendertarget);
	//glBindTexture(GL_TEXTURE_2D, rendertarget);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glGenTextures(1, &rin);
	//glBindTexture(GL_TEXTURE_2D, rin);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//// 创建深度缓冲区
	//glGenRenderbuffers(1, &renderbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//// 创建FBO对象
	//glGenFramebuffers(1, &framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendertarget, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glGenFramebuffers(1, &fbo2);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rin, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	testModel=new Model("Models/Shokaku/Shokaku.fbx");
	//testModel = new Model("Models/Nanosuit/nanosuit.obj");
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	GLchar* vertexShaderSource;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	//vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// 只需要绑定VBO不用再次设置VBO的数据，因为箱子的VBO数据中已经包含了正确的立方体顶点数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 设置灯立方体的顶点属性（对我们的灯来说仅仅只有位置数据）
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	

	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	
	
	glGenTextures(1, &texture);
	
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
 	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	
	
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data2 = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data2)
	{

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data2);
	//vertexShaderSource = (GLchar*)"#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main(){\n	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n}";
	//glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	//	cout << "ERROR::Vertex shader compile failed:" << infoLog << endl;
	//}

	//GLchar* fragmentShaderSource = (GLchar*)"#version 330 core\nout vec4 FragColor;\n	void main()\n{\nFragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n} ";
	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//glCompileShader(fragmentShader);

	//
	//if (!success)
	//{
	//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	//	cout << "ERROR::Fragment shader compile failed:" << infoLog << endl;
	//}
	//
	//shaderProgram = glCreateProgram();
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//glLinkProgram(shaderProgram);
	//glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	//if (!success) {
	//	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	//	cout << "ERROR::Link failed:" << infoLog << endl;
	//}
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);
	lampShader = new Shader("Shaders/lamp.vs", "Shaders/lamp.fs", nullptr, nullptr);
	ourShader = new Shader("Shaders/shader.vs", "Shaders/shader.fs", nullptr, nullptr);
	ourShader->use();
	glUniform1i(glGetUniformLocation(ourShader->ID, "texture1"), 0);
	ourShader->setInt("texture2", 1);
	ourShader->setFloat("iTime", 0.001f * clock());
	ourShader->setInt("RandSeed", rand());
	ourShader->setVec3("iResolution", glm::vec3(1600, 800, 1)); 

	diffuseShader = new Shader("Shaders/onlyDiffuse.vs", "Shaders/onlyDiffuse.fs", nullptr, nullptr);
	blinnPhongShader = new Shader("Shaders/blinnPhong.vs", "Shaders/blinnPhong.fs", nullptr, nullptr);
	simpleDepthShader = new Shader("Shaders/simpleDepthShader.vs", "Shaders/simpleDepthShader.fs", nullptr, nullptr);
	debugDepthQuad = new Shader("Shaders/debug_quad_depth.vs", "Shaders/debug_quad_depth.fs", nullptr, nullptr);
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(1600, 800, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create window" << endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to init  GLAD" << endl;
		return -1;
	}
	glViewport(0, 0, 1600, 800);
	init(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		render(window);
		glfwSwapBuffers(window);
		
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}