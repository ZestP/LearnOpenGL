#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <ctime>
#include "shader.h"
#define TEXTURE_WIDTH 1600
#define TEXTURE_HEIGHT 800
using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

 
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
float vertices[] = {
	0.5f, 0.5f, 0.0f, 1.0f,1.0f,  // 右上角
	0.5f, -0.5f, 0.0f,1.0f,0.0f,  // 右下角
	-0.5f, -0.5f, 0.0f,0.0f,0.0f, // 左下角
	-0.5f, 0.5f, 0.0f,0.0f,1.0f   // 左上角
};

unsigned int indices[] = { // 注意索引从0开始! 
	0, 1, 3, // 第一个三角形
	1, 2, 3  // 第二个三角形
};
unsigned int VBO;
unsigned int vertexShader;
unsigned int VAO;
unsigned int EBO;
unsigned int fragmentShader;
unsigned int shaderProgram;
unsigned int framebuffer,fbo2,renderbuffer, rendertarget,rin;
Shader* ourShader;
int cnt = 0;
void render(GLFWwindow* window)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	ourShader->use();
	

	
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ourShader->setFloat("iTime", 0.001f * clock());
	ourShader->setInt("RandSeed", rand());
	ourShader->setVec3("iResolution", glm::vec3(1600, 800,1));

	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);
	glBindVertexArray(0);


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
void init(GLFWwindow* window)
{
	srand((unsigned int)time(NULL));
	// 创建纹理
	glGenTextures(1, &rendertarget);
	glBindTexture(GL_TEXTURE_2D, rendertarget);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &rin);
	glBindTexture(GL_TEXTURE_2D, rin);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// 创建深度缓冲区
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// 创建FBO对象
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendertarget, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glGenFramebuffers(1, &fbo2);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rin, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	GLchar* vertexShaderSource;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	vertexShaderSource = (GLchar*)"#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main(){\n	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n}";
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::Vertex shader compile failed:" << infoLog << endl;
	}

	GLchar* fragmentShaderSource = (GLchar*)"#version 330 core\nout vec4 FragColor;\n	void main()\n{\nFragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n} ";
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::Fragment shader compile failed:" << infoLog << endl;
	}
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::Link failed:" << infoLog << endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
	ourShader=new Shader("shader.vs", "shader.fs");
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