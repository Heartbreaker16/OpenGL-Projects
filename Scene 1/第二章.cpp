#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model.h>
#include<math.h>
#include <mesh.h>

//夜色渐深（夜晚天空颜色值变化）
glm::vec3 colorbyTime(float time) {

	if (glfwGetTime() - time< 19)
		return glm::vec3((19 - glfwGetTime() + time) / 26, (19 - glfwGetTime() +time) / 26, (19 - (glfwGetTime() - time)*0.8905) / 26);
	else if (glfwGetTime() - time < 25 && glfwGetTime() - time >= 19)
		return glm::vec3(0, 0, 0.08);
	else
		return glm::vec3((glfwGetTime() - time - 25) / 40, (glfwGetTime() - time - 25) / 40, (glfwGetTime() - time - 21.8) / 40);
}

//摄像机相关参数
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
float cameraSpeed = 0.35f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//////////第一人称视角摄像机函数系列
//1.鼠标控制
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.03;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
//2.滚轮控制
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
		fov -= yoffset * 5;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 90.0f)
		fov = 90.0f;
}
//3.使用WSAD控制摄像机移动，Q减速，E加速
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed / (float)0.0166*deltaTime* cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed / (float)0.0166*deltaTime* cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed / (float)0.0166*deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed / (float)0.0166*deltaTime;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS&&cameraSpeed>=0.1)
		cameraSpeed -= 0.05f / (float)0.0166*deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraSpeed += 0.05f / (float)0.0166*deltaTime;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


//加载纹理
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	}
	else
		std::cout << "Texture failed to load at path: " << path << std::endl;
	stbi_image_free(data);
	return textureID;
}
//加载天空盒
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

//创建、链接着色器函数
void createProgram(const char* vs, const char* fs, unsigned int program, const char* gs = nullptr) {
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vs, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fs, NULL);
	glCompileShader(fragmentShader);

	unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	if (gs != nullptr) {
		glShaderSource(geometryShader, 1, &gs, NULL);
		glCompileShader(geometryShader);
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	if (gs != nullptr)
	glAttachShader(program, geometryShader);
	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);
}

//配置VAO函数
void LinkVertex(int length, int arrib2, int i) {
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, length * sizeof(float), (void*)0);
	if (i > 1)
	{
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, arrib2, GL_FLOAT, GL_FALSE, length * sizeof(float), (void*)(3 * sizeof(float)));
	}
	if (i > 2)
	{
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, length - (3 + arrib2), GL_FLOAT, GL_FALSE, length * sizeof(float), (void*)((3 + arrib2) * sizeof(float)));
	}
}

//立方体顶点
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

//天空盒顶点
float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};
//立方体位置
glm::vec3 cubePositions[] = {
	glm::vec3(-15.0f,  16.5f,  -90.0f),
	glm::vec3(15.0f,  16.5f, -90.0f),
	glm::vec3(45.0f,  16.5f,  -90.0f),
	glm::vec3(75.0f,  16.5f, -90.0f),
	glm::vec3(-45.0f,  16.5f,  -90.0f),
	glm::vec3(-75.0f,  16.5f, -90.0f),

	glm::vec3(-90.0f,  16.5f,  -45.0f),
	glm::vec3(-90.0f,  16.5f, 15.0f),
	glm::vec3(-90.0f,  16.5f,  45.0f),
	glm::vec3(-90.0f,  16.5f,  75.0f),
	glm::vec3(-90.0f,  16.5f,  -75.0f),
	glm::vec3(-90.0f,  16.5f,  -15.0f),

	glm::vec3(-15.0f,  16.5f,  90.0f),
	glm::vec3(15.0f,  16.5f, 90.0f),
	glm::vec3(45.0f,  16.5f,  90.0f),
	glm::vec3(75.0f,  16.5f, 90.0f),
	glm::vec3(-45.0f,  16.5f,  90.0f),
	glm::vec3(-75.0f,  16.5f, 90.0f),

	glm::vec3(90.0f,  16.5f,  -45.0f),
	glm::vec3(90.0f,  16.5f, 15.0f),
	glm::vec3(90.0f,  16.5f,  45.0f),
	glm::vec3(90.0f,  16.5f,  75.0f),
	glm::vec3(90.0f,  16.5f,  -75.0f),
	glm::vec3(90.0f,  16.5f,  -15.0f),
};
//点光源位置
glm::vec3 pointLightPositions[] = {
	glm::vec3(4.0f,  -2.0f,  8.0f),
	glm::vec3(-4.0f, -2.0f, 8.0f),
	glm::vec3(-4.0f,  2.0f, -8.0f),
	glm::vec3(4.0f,  2.0f, -8.0f),
	glm::vec3(4.0f,  2.0f,  8.0f),
	glm::vec3(-4.0f, 2.0f, 8.0f),
	glm::vec3(-4.0f,  -2.0f, -8.0f),
	glm::vec3(4.0f,  -2.0f, -8.0f),

};
//地面顶点
float planeVertices[] = {
	// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	90.0f, -1.51f,  90.0f,  20.0f, 0.0f,
	-90.0f, -1.51f,  90.0f,  0.0f, 0.0f,
	-90.0f, -1.51f, -90.0f,  0.0f, 20.0f,

	90.0f, -1.51f,  90.0f,  20.0f, 0.0f,
	-90.0f, -1.51f, -90.0f,  0.0f, 20.0f,
	90.0f, -1.51f, -90.0f, 0.0f,0.0f
};
//围墙顶点
float transparentVertices[] = {
	// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
	-10.0f,  -0.52 +300,  0.0f,  0.0f,  0.0f,
	-10.0f, -0.52,  0.0f,  0.0f,  1.0f,
	20.0f, -0.52,  0.0f,  1.0f,  1.0f,

	-10.0f,  -0.52 + 300,  0.0f,  0.0f,  0.0f,
	20.0f, -0.52,  0.0f,  1.0f,  1.0f,
	20.0f,  -0.52 + 300,  0.0f,  1.0f,  0.0f
};

//各种顶点、片源着色器源代码
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"out vec3 FragPos,Normal;\n"
"out vec2 TexCoords;\n"
"out VS_OUT{"
	"vec2 texCoords;"
"} vs_out;"
"uniform mat4 model,view, projection;"
"void main()"
"{"
"FragPos = vec3(model * vec4(aPos, 1.0));"
"Normal = mat3(transpose(inverse(model))) * aNormal;"
"TexCoords = aTexCoords;"
"vs_out.texCoords = aTexCoords;"
"gl_Position = projection * view * vec4(FragPos, 1.0f);"
"}";

const char* ReflectVertexSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 Position,Normal;\n"
"uniform mat4 model,view, projection;"
"void main()"
"{"
"Normal = mat3(transpose(inverse(model))) * aNormal;"
"Position = vec3(model * vec4(aPos, 1.0));"
"gl_Position = projection * view* model * vec4(aPos, 1.0f);"
"}";

const char*fragmentModelSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoords;\n"
"uniform sampler2D texture_diffuse1;"
"void main()"
"{"
"FragColor = texture(texture_diffuse1, TexCoords); "
"}\n";

const char*fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 FragPos,Normal;\n"
"in vec2 TexCoords;\n"

"uniform vec3 viewPos;"

"struct Material {"
"sampler2D diffuse;"
"sampler2D specular;"
"float shininess;"
"};"
"struct DirLight {"
"vec3 direction;"
"vec3 ambient;"
"vec3 diffuse;"
"vec3 specular;"
"};"
"struct PointLight {"
"float constant,linear,quadratic;"
"vec3 position;"
"vec3 ambient;"
"vec3 diffuse;"
"vec3 specular;"
"};"
"struct SpotLight {"
"float constant,linear,quadratic,cutOff,outerCutOff;"
"vec3 position;"
"vec3 ambient;"
"vec3 diffuse;"
"vec3 specular;"
"vec3 direction;"
"};"

"uniform Material material;"
"uniform DirLight dirLight;"
"uniform PointLight pointLights[8];"
"uniform SpotLight spotLight;"

"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)"
"{"
"vec3 lightDir = normalize(-light.direction);"
"	float diff = max(dot(normal, lightDir), 0.0);"
"	vec3 reflectDir = reflect(-lightDir, normal);"
"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
"	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));"
"	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));"
"  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));"
"	return (ambient + diffuse + specular);"
"}\n"
"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)"
"{"
"	vec3 lightDir = normalize(light.position - fragPos);"
"	float diff = max(dot(normal, lightDir), 0.0);"
"	vec3 reflectDir = reflect(-lightDir, normal);"
"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
"	float distance = length(light.position - fragPos);"
"	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));"
"	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));"
"	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));"
"	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));"
"	ambient *= attenuation;"
"	diffuse *= attenuation;"
"	specular *=8* attenuation;"
"	return (ambient + diffuse + specular);"
"}\n"
"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)"
"{"
"	vec3 lightDir = normalize(light.position - fragPos);"
"	float diff = max(dot(normal, lightDir), 0.0);"
"	vec3 reflectDir = reflect(-lightDir, normal);"
"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
"	float distance = length(light.position - fragPos);"
"	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));"
"	float theta = dot(lightDir, normalize(-light.direction));"
"	float epsilon = light.cutOff - light.outerCutOff;"
"	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);"
"	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));"
"	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));"
"	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));"
"	ambient *= attenuation * intensity;"
"	diffuse *= attenuation * intensity;"
"	specular *= attenuation * intensity;"
"	return (ambient + diffuse + specular);"
"}\n"

"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);"
"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);"
"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);"

"void main()"
"{"
"vec3 norm = normalize(Normal);"
"vec3 viewDir = normalize(viewPos - FragPos);"
"vec3 result = CalcDirLight(dirLight, norm, viewDir);"
"for(int i = 0; i < 8; i++)"
"result +=CalcPointLight(pointLights[i], norm, FragPos, viewDir);"
"result +=3* CalcSpotLight(spotLight, norm, FragPos, viewDir);"
"FragColor = vec4(result, 1.0); "
"}\n";

const char*lightingShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;"
"void main()"
"{"
"FragColor = vec4(color,1);"
"} ";

const char* vertexSKYSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 view, projection;"
"out vec3 TexCoords;\n"
"void main()"
"{"
"TexCoords =aPos;"
"vec4 pos = projection * view * vec4(aPos, 1.0);"
"gl_Position = pos.xyww;"
"}";

const char*lightingSKYSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 TexCoords;\n"
"uniform samplerCube skybox;"
"void main()"
"{"
"FragColor = texture(skybox, TexCoords);"
"} ";

const char*fragmentModelReflect = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal, Position;\n"
"in vec2 TexCoords;\n"
"uniform samplerCube texture_diffuse1;"
"uniform vec3 cameraPos;"
"uniform samplerCube skybox;"
"void main()"
"{"
"vec3 I = normalize(Position - cameraPos);"
"vec3 R =reflect(I, normalize(Normal));"
"FragColor = vec4(texture(skybox, R).rgb, 1.0); "
"}\n";

const char* vertexFloorSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoords;\n"
"out vec2 TexCoords;\n"
"uniform mat4 model,view, projection;"
"void main()"
"{"
"TexCoords = aTexCoords;"
"gl_Position = projection * view* model * vec4(aPos, 1.0f);"
"}";

const char*fragmentFloorSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoords;\n"
"uniform sampler2D texture1,texture2;"
"uniform float percent;"
"void main()"
"{"
"vec4 texColor = texture(texture1, TexCoords);"
"if(texColor.a < 0.1) discard;"
"FragColor = mix(texture(texture1, TexCoords), texture(texture2, TexCoords),percent);"
"}\n";

const char * geometrySource = "#version 330 core\n"
"layout(triangles) in; \n"
"layout(triangle_strip, max_vertices = 3) out; \n"
"in VS_OUT{ \n"
"	vec2 texCoords; \n"
"} gs_in[]; \n"
"out vec2 TexCoords; \n"
"uniform float time; \n"
"vec4 explode(vec4 position, vec3 normal)\n"
"{\n"
"	float magnitude = 30; \n"
"	vec3 direction = normal * ((sin(time) + 1.0)*(sin(time) + 1.0)*(sin(time) + 1.0) /10.0) * magnitude; \n"
"	return position + vec4(direction, 0.0); \n"
"}\n"
"vec3 GetNormal()\n"
"{\n"
"	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position); \n"
"	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position); \n"
"	return normalize(cross(a, b)); \n"
"}\n"
"void main() {\n"
"	vec3 normal = GetNormal(); \n"
"	gl_Position = explode(gl_in[0].gl_Position, normal); \n"
"	TexCoords = gs_in[0].texCoords; \n"
"EmitVertex(); \n"
"gl_Position = explode(gl_in[1].gl_Position, normal); \n"
"TexCoords = gs_in[1].texCoords; \n"
"EmitVertex(); \n"
"	gl_Position = explode(gl_in[2].gl_Position, normal); \n"
"	TexCoords = gs_in[2].texCoords; \n"
"	EmitVertex(); \n"
"	EndPrimitive(); \n"
"}\n";
const char* ReflectModelVS = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"out vec2 TexCoords;\n"
"out vec3 Normal, Position;\n"
"uniform mat4 trans,model,view, projection;"
"void main()"
"{"
"TexCoords = aTexCoords;"
"Normal = mat3(transpose(inverse(model))) * aNormal;"
"Position = vec3(model * vec4(aPos, 1.0));"
"gl_Position = projection * view * trans* model*vec4(aPos, 1.0f);"
"}";

const char*ReflectModelFS = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal, Position;\n"
"in vec2 TexCoords;\n"
"uniform sampler2D texture_diffuse1;"
"uniform vec3 cameraPos;"
"uniform samplerCube skybox;"
"void main()"
"{"
"vec3 I = normalize(Position - cameraPos);"
"vec3 R = reflect(I, normalize(Normal));"
"FragColor = vec4(texture(skybox, R).rgb, 1.0); "
"}\n";
//主函数
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1600, 1200, "Giant's Fly to the Moon", NULL, NULL);
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

	glViewport(0, 0, 1600,1200);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//创建、链接各种着色器 (ShaderProgram)
	unsigned int shaderProgram = glCreateProgram();
	createProgram(vertexShaderSource, fragmentModelSource, shaderProgram);
	unsigned int ReflectBox = glCreateProgram();
	createProgram(ReflectVertexSource, fragmentModelReflect, ReflectBox);
	unsigned int shaderProgramStart = glCreateProgram();
	createProgram(vertexShaderSource, fragmentShaderSource, shaderProgramStart);
	unsigned int lightingProgram = glCreateProgram();
	createProgram(vertexFloorSource, lightingShaderSource, lightingProgram);
	unsigned int skyProgram = glCreateProgram();
	createProgram(vertexSKYSource, lightingSKYSource, skyProgram);
	unsigned int modelReflect = glCreateProgram();
	createProgram(ReflectModelVS, ReflectModelFS, modelReflect);
	unsigned int floorProgram = glCreateProgram();
	createProgram(vertexFloorSource, fragmentFloorSource, floorProgram);
	unsigned int magicProgram = glCreateProgram();
	createProgram(vertexShaderSource, fragmentFloorSource, magicProgram);
	unsigned int explodeProgram = glCreateProgram();
	createProgram(vertexShaderSource, fragmentModelSource, explodeProgram, geometrySource);

	//创建各种VAO、VBO
	unsigned int  VAO,VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	LinkVertex(8,3,3);
	
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	LinkVertex(8, 3, 2);

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	LinkVertex(5,2,2);

	for (int i = 0; i < 30; i++)
	{
		if (planeVertices[i] == 20)
			planeVertices[i] = 1;
	}
	unsigned int planeVAO1, planeVBO1;
	glGenVertexArrays(1, &planeVAO1);
	glGenBuffers(1, &planeVBO1);
	glBindVertexArray(planeVAO1);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	LinkVertex(5, 2, 2);
	
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	LinkVertex(8,0,1);

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	LinkVertex(3,0,1);

	unsigned int transparentVAO, transparentVBO;
	glGenVertexArrays(1, &transparentVAO);
	glGenBuffers(1, &transparentVBO);
	glBindVertexArray(transparentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
	LinkVertex(5, 2, 2);

	//加载纹理与天空盒
	Model ourModel("D:\\Desktop\\ship\\ship_boat.obj");
	Model Moon("moon\\planet.obj");
	unsigned int diffuseMap = loadTexture("textures\\container2.png"); 
	unsigned int specularMap = loadTexture("textures\\container2_specular.png");
	unsigned int mixMap = loadTexture("textures\\container.png");
	unsigned int floorTexture = loadTexture("textures\\mofang.jpg");
	unsigned int floorTexture1 = loadTexture("skybox\\top.jpg");
	unsigned int transparentTexture = loadTexture("textures\\wall3.png");
	vector<std::string> faces	{
		"skybox\\right.jpg",
		"skybox\\left.jpg",
		"skybox\\top.jpg",
		"skybox\\bottom.jpg",
		"skybox\\back.jpg",
		"skybox\\front.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	glUseProgram(skyProgram);
	glUniform1i(glGetUniformLocation(skyProgram, "skybox"), 0);
	glUseProgram(modelReflect);
	glUniform1i(glGetUniformLocation(modelReflect, "skybox"), 0);
	glUseProgram(ReflectBox);
	glUniform1i(glGetUniformLocation(ReflectBox, "skybox"), 0);

	glm::mat4 view(1), projection(1);

	//开启深度测试
	glEnable(GL_DEPTH_TEST);
	
	//随机数产生星空和流星
	float x[200], y[200], timing = 0, lastTiming = 0,  xpos = 0;
	int  randNum=0, random[30];
	for (int i = 0; i < 200; i++) 
	{
		x[i] = -86 + rand() % 172;
		y[i] = rand() % 60;
		if (i < 30) random[i] = 40 + rand() % 70;
	}

	//场景中物体变换需要的变量
	bool play = false, start = true, stop = false, accelerate = false, allowed = true, explode = false, useReflect = false;
	float time = 0, speed = 1,modelAngle=0;
	glm::vec3 modelPos= glm::vec3(0,-7,0);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);		
		glm::mat4 model(1);
		projection = glm::perspective(glm::radians(fov), (float)800 / (float)600, 0.1f, 200.0f);
		//进入场景一
		if (start) {
			glClearColor(0,0,0,1);
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			float currentTime=0;
			//按下O 箱子变成魔方 并匀加速旋转
			if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
				accelerate = true;
				time = glfwGetTime();
			}
			if (accelerate) {
				currentTime = glfwGetTime() - time;
				//旋转5秒后画面变白
				if (currentTime > 5) {
					glClearColor(1, 1, 1, 1);
					allowed = false;
				}
				if (currentTime>6)
					stop = true;
			}
			if (!accelerate) {
				glUseProgram(shaderProgramStart);
				glUniform1i(glGetUniformLocation(shaderProgramStart, "material.diffuse"), 0);
				glUniform1i(glGetUniformLocation(shaderProgramStart, "material.specular"), 1);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, diffuseMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, specularMap);
				glUniformMatrix4fv(glGetUniformLocation(shaderProgramStart, "view"), 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(shaderProgramStart, "projection"), 1, GL_FALSE, &projection[0][0]);
				glUniform3fv(glGetUniformLocation(shaderProgramStart, "light.position"), 1, glm::value_ptr(lightPos));
				glUniform3fv(glGetUniformLocation(shaderProgramStart, "viewPos"), 1, glm::value_ptr(cameraPos));
				glUniform1f(glGetUniformLocation(shaderProgramStart, "material.shininess"), 200);

				glUniform3f(glGetUniformLocation(shaderProgramStart, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

				//箱子受到点光源的照射，且箱子表面不同材质有不同程度的反光
				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[0].position"), 1, glm::value_ptr(pointLightPositions[0]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[0].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[0].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[0].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[1].position"), 1, glm::value_ptr(pointLightPositions[1]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[1].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[1].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[1].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[1].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[2].position"), 1, glm::value_ptr(pointLightPositions[2]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[2].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[2].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[2].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[2].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[3].position"), 1, glm::value_ptr(pointLightPositions[3]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[3].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[3].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[3].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[3].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[3].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[3].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[4].position"), 1, glm::value_ptr(pointLightPositions[4]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[4].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[4].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[4].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[4].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[4].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[4].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[5].position"), 1, glm::value_ptr(pointLightPositions[5]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[5].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[5].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[5].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[5].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[5].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[5].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[6].position"), 1, glm::value_ptr(pointLightPositions[6]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[6].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[6].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[6].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[6].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[6].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[6].quadratic"), 0.032f);

				glUniform3fv(glGetUniformLocation(shaderProgramStart, "pointLights[7].position"), 1, glm::value_ptr(pointLightPositions[7]));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[7].ambient"), 0.05f, 0.05f, 0.05f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[7].diffuse"), 0.8f, 0.8f, 0.8f);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "pointLights[7].specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[7].constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[7].linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "pointLights[7].quadratic"), 0.032f);
				//箱子受到手电筒（聚光灯）照射
				glUniform3fv(glGetUniformLocation(shaderProgramStart, "spotLight.position"), 1, glm::value_ptr(cameraPos));
				glUniform3fv(glGetUniformLocation(shaderProgramStart, "spotLight.direction"), 1, glm::value_ptr(cameraFront));
				glUniform3f(glGetUniformLocation(shaderProgramStart, "spotLight.ambient"), 0, 0, 0);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "spotLight.diffuse"), 1, 1, 1);
				glUniform3f(glGetUniformLocation(shaderProgramStart, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "spotLight.constant"), 1);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "spotLight.linear"), 0.09f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "spotLight.quadratic"), 0.032f);
				glUniform1f(glGetUniformLocation(shaderProgramStart, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
				glUniform1f(glGetUniformLocation(shaderProgramStart, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));
				glUniformMatrix4fv(glGetUniformLocation(shaderProgramStart, "model"), 1, GL_FALSE, &model[0][0]);
			}
			else if(currentTime<4) {
				glUseProgram(magicProgram);
				glUniform1i(glGetUniformLocation(magicProgram, "texture1"), 0);
				glUniform1i(glGetUniformLocation(magicProgram, "texture2"), 1);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mixMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, floorTexture);
				glUniform1f(glGetUniformLocation(magicProgram, "percent"), currentTime*currentTime /15);
				glUniformMatrix4fv(glGetUniformLocation(magicProgram, "view"), 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(magicProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
			}
			else {
				glUseProgram(explodeProgram);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, floorTexture);
				glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "view"), 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
				glUniform1f(glGetUniformLocation(explodeProgram, "time"), (currentTime-4)*2.5-glm::radians(90.0));
			}
			glBindVertexArray(VAO);
					
			model = glm::translate(model, glm::vec3(0,0,6));
			if(currentTime<4)
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
			else model = glm::rotate(model, time+4, glm::vec3(1.0f, 0.3f, 0.5f));
			model = glm::scale(model, glm::vec3(2.7f));
			if(accelerate)
				glUniformMatrix4fv(glGetUniformLocation(magicProgram, "model"), 1, GL_FALSE, &model[0][0]);
			else if(currentTime<4)
				glUniformMatrix4fv(glGetUniformLocation(shaderProgramStart, "model"), 1, GL_FALSE, &model[0][0]);
			else 
				glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "model"), 1, GL_FALSE, &model[0][0]);
			if(allowed)
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//魔方旋转5秒后切换入另一场景，并将摄像机位置还原至初始值
		if (stop) 
		{ 
			lastX = 400, lastY = 300;
			firstMouse = true;
			yaw = -90.0f;	
			pitch = 0.0f;
			fov = 45.0f;
			cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
			cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
			cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
			cameraDirection = glm::normalize(cameraPos - cameraTarget);
			up = glm::vec3(0.0f, 1.0f, 0.0f);
			cameraRight = glm::normalize(glm::cross(up, cameraDirection));
			cameraUp = glm::cross(cameraDirection, cameraRight);
			time = glfwGetTime();
			time = time -5;
			play = true;
			start = false;
			stop = false;
		}
		//魔方被开启，进入场景二——魔方内部的世界
		if (play) 
		{
			//月亮升起、夜色渐深的动画效果
			glUseProgram(shaderProgram);
			if (glfwGetTime()-time < 19)
			{
				glClearColor((19 - glfwGetTime()+time) / 26, (19 - glfwGetTime() + time) / 26, (19 - (glfwGetTime() - time)*0.8905) / 26, 1);
				model = glm::translate(model, glm::vec3(0, ((float)glfwGetTime() -time - 11) * 5, -94));
			}
			else if (glfwGetTime() - time < 24 && glfwGetTime() - time >= 19)
			{
				glClearColor(0, 0, 0.08, 1);
				model = glm::translate(model, glm::vec3(0, 40, -94));
			}
			else
			{
				//明月当空，夜色正浓
				if (glfwGetTime() - time < 25)
					glClearColor(0, 0, 0.08, 1);
				else
				//月亮落下，黎明将至
					glClearColor((glfwGetTime() - time - 25) / 40, (glfwGetTime() - time - 25) / 40, (glfwGetTime() - time - 21.8) / 40, 1);
				model = glm::translate(model, glm::vec3(0, 40 - ((float)glfwGetTime() - time - 24) * 5, -94));
			}
			model= glm::scale(model, glm::vec3(1.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

			if (40 - ((float)glfwGetTime() - time - 24) * 5 > 0)
				Moon.Draw(shaderProgram);
			else
			{
				//白天呈现天空盒效果
				glClearColor(0, 0, 0, 1);
				glDepthFunc(GL_LEQUAL);
				glUseProgram(skyProgram);
				view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
				glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, &view[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
				glBindVertexArray(skyboxVAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
				glDepthFunc(GL_LESS);
				useReflect = true;
			}
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, floorTexture);
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

			glUseProgram(ReflectBox);
			glUniformMatrix4fv(glGetUniformLocation(ReflectBox, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(ReflectBox, "projection"), 1, GL_FALSE, &projection[0][0]);
			glUniform3fv(glGetUniformLocation(ReflectBox, "cameraPos"), 1, glm::value_ptr(cameraPos));

			glUseProgram(explodeProgram);
			glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

			glUseProgram(shaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
			
			//围墙上方转动的立方体
			for (unsigned int i = 0; i < 24; i++)
			{
				//夜晚立方体呈现魔方效果
				glm::mat4 model(1);
				model = glm::translate(model, cubePositions[i]);
				float angle = 20.0f * i;
				model = glm::rotate(model, glm::radians(angle) + (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
				model = glm::scale(model, glm::vec3(2.7f));
				if (!useReflect) {
					glBindVertexArray(VAO);
					glUseProgram(shaderProgram);
					glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
				}
				else {
					//白天立方体呈现镜子效果
					glBindVertexArray(0);
					glUseProgram(ReflectBox);
					glBindVertexArray(cubeVAO);
					glActiveTexture(GL_TEXTURE0);
					glUniformMatrix4fv(glGetUniformLocation(ReflectBox, "model"), 1, GL_FALSE, &model[0][0]);
					glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
				}
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

			glUseProgram(lightingProgram);
			glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
			glBindVertexArray(lightVAO);

			timing = lastTiming;
			lastTiming+=deltaTime/0.0166;
			if (lastTiming > 200) lastTiming = 0;

			//随机产生的闪烁的星空效果
			if (40 - ((float)glfwGetTime() - time - 24) * 5 > 0)
			{
				for (unsigned int i = 0; i < 200; i++)
				{
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(x[i], y[i], -96));
					model = glm::scale(model, glm::vec3(0.2f)); 
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					timing++;
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(x[i], y[i], 96));
					model = glm::scale(model, glm::vec3(0.2f)); 
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					timing++;
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(-96, y[i], x[i]));
					model = glm::scale(model, glm::vec3(0.2f)); 
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					timing++;
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(96, y[i], x[i]));
					model = glm::scale(model, glm::vec3(0.2f));
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					timing++;
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(y[i], 50, x[i]));
					model = glm::scale(model, glm::vec3(0.2f));
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);

					timing++;
					if (timing > 200) timing = 0;
					glUniform3f(glGetUniformLocation(lightingProgram, "color"), timing / 15 + colorbyTime(time).x, timing / 15 + colorbyTime(time).y, timing / 15 + colorbyTime(time).z);
					model = glm::mat4(1);
					model = glm::translate(model, glm::vec3(-y[200 - i], 50, x[i]));
					model = glm::scale(model, glm::vec3(0.2f)); 
					glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
			xpos+=deltaTime/0.0166;

			//随机产生的流星效果
			if (random[randNum] - 2 * xpos > 0 && 40 - ((float)glfwGetTime() - time - 24) * 5 > 0)
			{
				glUniform3f(glGetUniformLocation(lightingProgram, "color"), 1, 1, 1);
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-2 * xpos + 90, random[randNum] - 2 * xpos, -91));
				model = glm::scale(model, glm::vec3(0.5f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-2 * xpos + 10, random[randNum] - 2 * xpos, -91));
				model = glm::scale(model, glm::vec3(0.5f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(2 * xpos - 80, random[randNum] - 2 * xpos, 91));
				model = glm::scale(model, glm::vec3(0.4f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(2 * xpos, random[randNum] - 2 * xpos, 91));
				model = glm::scale(model, glm::vec3(0.4f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-100, random[randNum] - 2 * xpos, 2 * xpos - 91));
				model = glm::scale(model, glm::vec3(0.4f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(100, random[randNum] - 2 * xpos, -2 * xpos + 91));
				model = glm::scale(model, glm::vec3(0.4f));
				glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			else {
				randNum++; 
				xpos = 0;
				if (randNum > 28) randNum = 0;
			};

			glUseProgram(modelReflect);
			glUniformMatrix4fv(glGetUniformLocation(modelReflect, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(modelReflect, "projection"), 1, GL_FALSE, &projection[0][0]);

			//巨人模型碎片合体，旋转后再按照Z字形线路飞天，各种变换组合成的奔月动画
			model = glm::mat4(1);
			glUseProgram(shaderProgram);
			glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, 0);
			model = glm::scale(model, glm::vec3(0.2f));
			model = glm::translate(model, glm::vec3(0, -7, 0));

			if (glfwGetTime() - time> 8 && glfwGetTime() - time < 11.011246)
				model = glm::rotate(model, (float)((glfwGetTime() - time - 8)*(glfwGetTime() - time - 8)), glm::vec3(0.0f, 1.0f, 0.0f));
			else if(glfwGetTime() - time> 11.011246 && glfwGetTime() - time < 16.06)
				model = glm::rotate(model, 3.1415926f-(float)((glfwGetTime() - time - 16.06)*(glfwGetTime() - time - 16.06)), glm::vec3(0.0f, 1.0f, 0.0f));
			else if (glfwGetTime() - time > 16.06 && glfwGetTime() - time < 17) {
				model = glm::rotate(model, (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, (float)((glfwGetTime() - time)* 1.671 / 2 - 7.134948), glm::vec3(1.0f, 0.0f, 0.0f));
			}
			else if (glfwGetTime() - time > 17 && glfwGetTime() - time< 20) {
				model = glm::mat4(1);
				model = glm::scale(model, glm::vec3(0.2f));
				model = glm::rotate(model, (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, (float)(7.068552), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3((glfwGetTime() - time - 17) * 20, (glfwGetTime() - time - 17) * 70, (glfwGetTime() - time - 17) * 190 / 7));
			}
			else if (glfwGetTime() - time > 20 && glfwGetTime() - time < 22) {
				model = glm::mat4(1);
				model = glm::scale(model, glm::vec3(0.2f));
				model = glm::rotate(model, (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, (float)(7.068552), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(-(glfwGetTime() - time - 21.5) * 40, (glfwGetTime() - time - 17) * 70, (glfwGetTime() - time - 17) * 190 / 7));			
			}
			else if (glfwGetTime() - time > 22 && glfwGetTime() - time < 23.5) {
				model = glm::mat4(1);
				model = glm::scale(model, glm::vec3(0.2f));
				model = glm::rotate(model, (float)3.1415926, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, (float)(7.068552), glm::vec3(1.0f, 0.0f, 0.0f));
				//即将飞天前匀减速旋转
				model = glm::translate(model, glm::vec3((glfwGetTime() - time - 24) * 10, (glfwGetTime() - time - 17) * 70, (glfwGetTime() - time - 17) * 190 / 7));
			}
			else if (glfwGetTime() - time > 23.5) {
				//奔月结束后 可以使用方向键和H N键控制巨人移动
				model = glm::mat4(1);
				model = glm::scale(model, glm::vec3(0.2f));
				float moveSpeed = 1;
				if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
					model = glm::translate(model, modelPos+glm::vec3(0, moveSpeed, 0));
					modelPos += glm::vec3(0, moveSpeed, 0);
				}
				else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
					model = glm::translate(model, modelPos + glm::vec3(0, -moveSpeed, 0));
					modelPos += glm::vec3(0, -moveSpeed, 0);
				}
				else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
					model = glm::translate(model, modelPos + glm::vec3(-moveSpeed, 0, 0));
					modelPos += glm::vec3(-moveSpeed, 0, 0);
				}
				else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
					model = glm::translate(model, modelPos + glm::vec3(moveSpeed, 0, 0));
					modelPos += glm::vec3(moveSpeed, 0, 0);
				}
				else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
					model = glm::translate(model, modelPos + glm::vec3(0, 0, -moveSpeed));
					modelPos += glm::vec3(0, 0, -moveSpeed);
				}
				else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
					model = glm::translate(model, modelPos + glm::vec3(0, 0, moveSpeed));
					modelPos += glm::vec3(0, 0, moveSpeed);
				}
				else
					model = glm::translate(model, modelPos);				
			}
			if (glfwGetTime() - time<12.5) {
				//巨人身体的碎片重组
				glUseProgram(explodeProgram);
				glUniform1f(glGetUniformLocation(explodeProgram, "time"),(glfwGetTime() - time-2)/10.5*glm::radians(180.0)+ glm::radians(90.0));
				glUniformMatrix4fv(glGetUniformLocation(explodeProgram, "model"), 1, GL_FALSE, &model[0][0]);
				ourModel.Draw(explodeProgram);
			}
			else if (!useReflect) 
			{
				glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
				ourModel.Draw(shaderProgram);
			}
			else
			{
				//巨人的身体在白天会变成映射天空的镜子			
					glUseProgram(modelReflect);
					glUniform3fv(glGetUniformLocation(modelReflect, "cameraPos"), 1, glm::value_ptr(cameraPos));
					glm::mat4 trans(1);
					trans = glm::translate(trans, glm::vec3(0, 0, 0));
					glUniformMatrix4fv(glGetUniformLocation(modelReflect, "trans"), 1, GL_FALSE, glm::value_ptr(trans));
					glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
					glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, &model[0][0]);
					ourModel.Draw(modelReflect);
			}
			//绘制地面
			glUseProgram(floorProgram);
			glUniformMatrix4fv(glGetUniformLocation(floorProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(floorProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
			model = glm::mat4(1);
			
			if (40 - ((float)glfwGetTime() - time - 24) * 5 > 0) {
				glBindVertexArray(planeVAO);
				glBindTexture(GL_TEXTURE_2D, floorTexture);
			}
			else {
				glBindVertexArray(planeVAO1);
				glBindTexture(GL_TEXTURE_2D, floorTexture1);
			}
			glUniformMatrix4fv(glGetUniformLocation(floorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			//绘制四面围栏
			glUseProgram(floorProgram);
			glBindVertexArray(transparentVAO);
			glBindTexture(GL_TEXTURE_2D, transparentTexture);
			for (int i = 0; i < 180; i = i + 30)
			{
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(i - 80, -1.51, -90));
				glUniformMatrix4fv(glGetUniformLocation(floorProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(i - 80, -1.51, 90));
				glUniformMatrix4fv(glGetUniformLocation(floorProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(90, -1.51, i - 80));
				model = glm::rotate(model, -(float)3.1415926 / 2, glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(glGetUniformLocation(floorProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-90, -1.51, i - 80));
				model = glm::rotate(model, -(float)3.1415926 / 2, glm::vec3(0.0f, 1.0f, 0.0f));
				glUniformMatrix4fv(glGetUniformLocation(floorProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
	}
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	glfwTerminate();
	return 0;
}