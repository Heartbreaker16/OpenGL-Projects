#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model.h>
#include<math.h>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float fov = 45.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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

	float sensitivity = 0.05;
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
		fov -= yoffset * 5;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 90.0f)
		fov = 90.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 0.35f;// adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

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

float planeVertices[] = {
	// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	30.0f, -1, 30.0f,  10.0f, 0.0f,
	-30.0f,-1,  30.0f,  0.0f, 0.0f,
	-30.0f, -1, -60.0f,  0.0f, 10.0f,

	30.0f, -1,  30.0f,  0.0f,10.0f,
	-30.0f, -1, -60.0f,  10.0f, 0.0f,
	30.0f, -1, -60.0f, 0.0f, 0.0f
};

float transparentVertices[] = {
	// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
	-10.0f,  10 - 1,  -10.0f,  0.0f,  0.0f,
	-10.0f, 0.01 - 1,  -10.0f,  0.0f,  1.0f,
	-10.0f, 0.01 - 1,  10.0f,  1.0f,  1.0f,

	-10.0f,  10 - 1,  -10.0f,  0.0f,  0.0f,
	-10.0f, 0.01 - 1,  10.0f,  1.0f,  1.0f,
	-10.0f,  10 - 1,  10.0f,  1.0f,  0.0f
};

float mubuVertices[] = {
	// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
	-10.0f,  10 - 1,  -10.0f,  0.0f,  0.0f,
	-10.0f, 0.01 - 11,  -10.0f,  0.0f,  1.0f,
	-10.0f, 0.01 - 11,  10.0f,  1.0f,  1.0f,

	-10.0f,  10 - 1,  -10.0f,  0.0f,  0.0f,
	-10.0f, 0.01 -11,  10.0f,  1.0f,  1.0f,
	-10.0f,  10 - 1,  10.0f,  1.0f,  0.0f
};

float billboard[] = {
	-7.0f,  7 - 1,  0.0f,  
	-7.0f,3,  -0.0f,
	7.0f, 3,  -0.0f,

	7, 3,  -0.0f,
	7, 7- 1,  -0.0f,
	-7,  7 - 1,  -0.0f
};

float colortop[] = {
	// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
	10.0f, 10.0 - 1,  10.0f,  0,0,0,
	-10.0f,10 - 1,  10.0f, 0,0,0,
	-10.0f, 10 - 1, -10.0f,  0,0,0,
	10.0f, 10 - 1, -10.0f, 0,0,0
};

float inside[] = {
	// positions        
	-10.0f, 0,  -1.0f, 
	-10.0f,0,  1.0f, 
	-10.0f, 1.732, 0
};

float colorvertices[] = {
	// positions                   // colors
	-10.0f,  10 - 1,  -10.0f,  0,0,0,
	-10.0f, 0.01 - 1,  -10.0f,  0,0,0,
	-10.0f, 0.01 - 1,  10.0f,  0,0,0,
	-10.0f,  10 - 1,  10.0f,  0,0,0
};

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

float vertices[] = {
	// positions          // normals           // texture coords
	-10.0f, 9.0f, -10.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
	10.0f, 9.0f, -10.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
	10.0f, 9.0f,  10.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	10.0f, 9.0f,  10.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	-10.0f,9.0f,  10.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
	-10.0f, 9.0f, -10.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
};
unsigned int indices[] = { // 廣吭沫哈貫0蝕兵! 
	0, 1, 3, // 及匯倖眉叔侘
	1, 2, 3  // 及屈倖眉叔侘
};

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoords;\n"
"out vec2 TexCoords;\n"
"uniform mat4 model,view, projection;"
"void main()"
"{"
"TexCoords = aTexCoords;"
"gl_Position = projection * view* model * vec4(aPos, 1.0f);"
"}";

const char*fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoords;\n"
"uniform sampler2D texture1;"
"void main()"
"{"
"vec4 texColor = texture(texture1, TexCoords);"
"if(texColor.a < 0.1) discard;"
"FragColor = texture(texture1, TexCoords); "
"}\n";

const char* vertexLightningShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 view, projection;"
"out vec3 TexCoords;\n"
"void main()"
"{"
"TexCoords =aPos;"
"vec4 pos = projection * view * vec4(aPos, 1.0);"
"gl_Position = pos.xyww;"
"}";

const char*lightingShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 TexCoords;\n"
"uniform samplerCube skybox;"
"void main()"
"{"
"FragColor = texture(skybox, TexCoords);"
"} ";

const char* lightVS = "#version 330 core\n"
"layout(location = 0) in vec3 aPos; \n"
"uniform mat4 model, view, projection;\n"
"void main()\n"
"{gl_Position = projection * view * model * vec4(aPos, 1.0);}\n";

const char* lightFS = "#version 330 core\n"
"uniform vec4 color; \n"
"out vec4 FragColor; \n"
"void main()\n"
"{FragColor = color; }\n";

const char* vertexModelSource = "#version 330 core\n"
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
"gl_Position = projection * view * trans * model*vec4(aPos, 1.0f);"
"}";

const char*fragmentModelReflect = "#version 330 core\n"
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

const char *colorVertex = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform mat4 model, view, projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model *vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}";

const char *colorFragment = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}";

const char* cubeVS = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"layout(location = 2) in vec2 aTexCoords;\n"
"out vec3 FragPos, Normal;\n"
"out vec2 TexCoords;\n"
"uniform mat4 model, view, projection;\n"
"void main()\n"
"{"
"	FragPos = vec3(model * vec4(aPos, 1.0));"
"	Normal = mat3(transpose(inverse(model))) * aNormal;"
"	TexCoords = aTexCoords;"
"	gl_Position = projection * view * vec4(FragPos, 1.0);"
"}";

const char* cubeFS = " #version 330 core\n"
"out vec4 FragColor; \n"

"struct Material {\n"
"sampler2D diffuse; \n"
"sampler2D specular; \n"
"	float shininess; \n"
"}; \n"

"struct SpotLight {\n"
"	vec3 position; \n"
"vec3 direction; \n"
"	float cutOff; \n"
"float outerCutOff; \n"
"float constant; \n"
"float linear; \n"
"float quadratic; \n"
"vec3 ambient; \n"
"vec3 diffuse; \n"
"vec3 specular; \n"
"}; \n"

"in vec3 FragPos; \n"
"in vec3 Normal; \n"
"in vec2 TexCoords; \n"

"uniform vec3 viewPos; \n"
"uniform SpotLight spotLight; \n"
"uniform Material material; \n"

"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir); \n"

"void main()\n"
"{\n"
"vec3 norm = normalize(Normal); \n"
"vec3 viewDir = normalize(viewPos - FragPos); \n"
"vec3 result =2*CalcSpotLight(spotLight, norm, FragPos, viewDir); \n"
"FragColor = vec4(result, 1.0); \n"
"}\n"

"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
"{\n"
"vec3 lightDir = normalize(light.position - fragPos); \n"
"float diff = max(dot(normal, lightDir), 0.0); \n"
"vec3 reflectDir = reflect(-lightDir, normal); \n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); \n"
"float distance = length(light.position - fragPos); \n"
"float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); \n"
"float theta = dot(lightDir, normalize(-light.direction)); \n"
"float epsilon = light.cutOff - light.outerCutOff; \n"
"float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); \n"
"vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords)); \n"
"vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)); \n"
"vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)); \n"
"ambient *= attenuation * intensity; \n"
"diffuse *= attenuation * intensity; \n"
"	specular *= attenuation * intensity; \n"
"return (ambient + diffuse + specular); \n"
"}\n";

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyOpenGL", NULL, NULL);
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_DEPTH_TEST);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int vertexLightShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexLightShader, 1, &vertexLightningShaderSource, NULL);
	glCompileShader(vertexLightShader);

	unsigned int lightingShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lightingShader, 1, &lightingShaderSource, NULL);
	glCompileShader(lightingShader);

	unsigned int lampvs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(lampvs, 1, &lightVS, NULL);
	glCompileShader(lampvs);

	unsigned int lampfs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lampfs, 1, &lightFS, NULL);
	glCompileShader(lampfs);

	unsigned int vertexModelShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexModelShader, 1, &vertexModelSource, NULL);
	glCompileShader(vertexModelShader);

	unsigned int ReflectModelShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ReflectModelShader, 1, &fragmentModelReflect, NULL);
	glCompileShader(ReflectModelShader);

	int colorvertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(colorvertex, 1, &colorVertex, NULL);
	glCompileShader(colorvertex);

	int colorfragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(colorfragment, 1, &colorFragment, NULL);
	glCompileShader(colorfragment);

	int colorProgram = glCreateProgram();
	glAttachShader(colorProgram, colorvertex);
	glAttachShader(colorProgram, colorfragment);
	glLinkProgram(colorProgram);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	unsigned int lightingProgram = glCreateProgram();
	glAttachShader(lightingProgram, vertexLightShader);
	glAttachShader(lightingProgram, lightingShader);
	glLinkProgram(lightingProgram);

	unsigned int lampProgram = glCreateProgram();
	glAttachShader(lampProgram, lampvs);
	glAttachShader(lampProgram, lampfs);
	glLinkProgram(lampProgram);

	unsigned int modelReflect = glCreateProgram();
	glAttachShader(modelReflect, vertexModelShader);
	glAttachShader(modelReflect, ReflectModelShader);
	glLinkProgram(modelReflect);

	unsigned int cubevs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(cubevs, 1, &cubeVS, NULL);
	glCompileShader(cubevs);

	unsigned int cubefs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(cubefs, 1, &cubeFS, NULL);
	glCompileShader(cubefs);

	unsigned int cubeProgram = glCreateProgram();
	glAttachShader(cubeProgram, cubevs);
	glAttachShader(cubeProgram, cubefs);
	glLinkProgram(cubeProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(lightingShader);

	unsigned int colorVBO, colorVAO,ebo,colorTopVAO,colorTopVBO;
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int transparentVAO, transparentVBO;
	glGenVertexArrays(1, &transparentVAO);
	glGenBuffers(1, &transparentVBO);
	glBindVertexArray(transparentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int mubuVAO, mubuVBO;
	glGenVertexArrays(1, &mubuVAO);
	glGenBuffers(1, &mubuVBO);
	glBindVertexArray(mubuVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mubuVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mubuVertices), &mubuVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int billboardVAO, billboardVBO;
	glGenVertexArrays(1, &billboardVAO);
	glGenBuffers(1, &billboardVBO);
	glBindVertexArray(billboardVAO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboard), &billboard, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	unsigned int insideVBO, insideVAO;
	glGenVertexArrays(1, &insideVAO);
	glGenBuffers(1, &insideVBO);
	glBindVertexArray(insideVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(inside), inside, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int floor = loadTexture("Desktop\\zhuan.jpg");
	unsigned int cell = loadTexture("Desktop\\zhuan3.jpg");

	Model ourModel("nanosuit\\nanosuit.obj");
	
	unsigned int floorTexture = loadTexture("Desktop\\345.jpg");
	unsigned int transparentTexture = loadTexture("Desktop\\mb.png");
	unsigned int cinema = loadTexture("Desktop\\cinema.png");

	//Tom & Jerry
	unsigned int f1= loadTexture("Capture\\1.jpg");
	unsigned int f2 = loadTexture("Capture\\2.jpg");
	unsigned int f3 = loadTexture("Capture\\3.jpg");
	unsigned int f4 = loadTexture("Capture\\4.jpg");
	unsigned int f5 = loadTexture("Capture\\5.jpg");
	unsigned int f6 = loadTexture("Capture\\6.jpg");
	unsigned int f7 = loadTexture("Capture\\7.jpg");
	unsigned int f8 = loadTexture("Capture\\8.jpg");
	unsigned int f9 = loadTexture("Capture\\9.jpg");
	unsigned int f10 = loadTexture("Capture\\10.jpg");
	unsigned int f11 = loadTexture("Capture\\11.jpg");
	unsigned int f12 = loadTexture("Capture\\12.jpg");
	unsigned int f13 = loadTexture("Capture\\13.jpg");
	unsigned int f14 = loadTexture("Capture\\14.jpg");
	unsigned int f15 = loadTexture("Capture\\15.jpg");
	unsigned int f16 = loadTexture("Capture\\16.jpg");
	unsigned int f17 = loadTexture("Capture\\17.jpg");
	unsigned int f18 = loadTexture("Capture\\18.jpg");
	unsigned int f19 = loadTexture("Capture\\19.jpg");
	unsigned int f20 = loadTexture("Capture\\20.jpg");
	unsigned int f21 = loadTexture("Capture\\21.jpg");
	unsigned int f22 = loadTexture("Capture\\22.jpg");
	unsigned int f23 = loadTexture("Capture\\23.jpg");
	unsigned int f24 = loadTexture("Capture\\24.jpg");
	unsigned int f25 = loadTexture("Capture\\25.jpg");
	unsigned int f26= loadTexture("Capture\\26.jpg");
	unsigned int f27 = loadTexture("Capture\\27.jpg");
	unsigned int f28 = loadTexture("Capture\\28.jpg");
	unsigned int f29 = loadTexture("Capture\\29.jpg");
	unsigned int f30 = loadTexture("Capture\\30.jpg");
	unsigned int f31 = loadTexture("Capture\\31.jpg");
	unsigned int f32 = loadTexture("Capture\\32.jpg");
	unsigned int f33 = loadTexture("Capture\\33.jpg");
	unsigned int f34 = loadTexture("Capture\\34.jpg");

	vector<std::string> faces
	{
		"skybox\\right.jpg",
		"skybox\\left.jpg",
		"skybox\\top.jpg",
		"skybox\\bottom.jpg",
		"skybox\\back.jpg",
		"skybox\\front.jpg"
	};
	vector<std::string> faces1
	{
		"Desktop\\sb_frozen\\frozen_rt.tga",
		"Desktop\\sb_frozen\\frozen_lf.tga",
		"Desktop\\sb_frozen\\frozen_up.tga",
		"Desktop\\sb_frozen\\frozen_dn.tga",
		"Desktop\\sb_frozen\\frozen_bk.tga",
		"Desktop\\sb_frozen\\frozen_ft.tga"
	};
	vector<std::string> faces2
	{
		"Desktop\\mp_alpha\\alpha-island_rt.tga",
		"Desktop\\mp_alpha\\alpha-island_lf.tga",
		"Desktop\\mp_alpha\\alpha-island_up.tga",
		"Desktop\\mp_alpha\\alpha-island_dn.tga",
		"Desktop\\mp_alpha\\alpha-island_bk.tga",
		"Desktop\\mp_alpha\\alpha-island_ft.tga"
	};
	vector<std::string> faces3
	{
		"Desktop\\mp_fcih\\fat-chance-in-hell_rt.tga",
		"Desktop\\mp_fcih\\fat-chance-in-hell_lf.tga",
		"Desktop\\mp_fcih\\fat-chance-in-hell_up.tga",
		"Desktop\\mp_fcih\\fat-chance-in-hell_dn.tga",
		"Desktop\\mp_fcih\\fat-chance-in-hell_bk.tga",
		"Desktop\\mp_fcih\\fat-chance-in-hell_ft.tga"
	};
	vector<std::string> faces4
	{
		"redsky\\right.jpg",
		"redsky\\left.jpg",
		"redsky\\top.jpg",
		"redsky\\bottom.jpg",
		"redsky\\back.jpg",
		"redsky\\front.jpg"
	};

	unsigned int cubemapTexture = loadCubemap(faces4);
	unsigned int man1=loadCubemap(faces1);
	unsigned int man2 = loadCubemap(faces2);
	unsigned int man3 = loadCubemap(faces3);
	unsigned int man4 = loadCubemap(faces);
	bool feature1 = false, feature2 = false, feature3 = false, feature4 = false, feature5 = false, playfilm = false;
	float i = 9999, time = -10000, time1 = -10000, time2 = -10000, time3 = -10000, time4 = -10000, film = 0,filmStartTime=0;
	bool process = false;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	while (!glfwWindowShouldClose(window))
	{	
		i--;
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view(1);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection(1);	
		projection = glm::perspective(glm::radians(fov), (float)128/ (float)72, 0.1f, 100.0f);
		glm::mat4 model(1);

		glUseProgram(cubeProgram);
		glUniform3fv(glGetUniformLocation(cubeProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
		glUniform1f(glGetUniformLocation(cubeProgram, "material.shininess"), 32);

		glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.position"), 1, glm::value_ptr(cameraPos));
		glUniform3fv(glGetUniformLocation(cubeProgram, "spotLight.direction"), 1, glm::value_ptr(cameraFront));
		glUniform3f(glGetUniformLocation(cubeProgram, "spotLight.ambient"), 0, 0, 0);
		glUniform3f(glGetUniformLocation(cubeProgram, "spotLight.diffuse"), 1, 1, 1);
		glUniform3f(glGetUniformLocation(cubeProgram, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.constant"), 1);
		glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.quadratic"), 0.016f);
		glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(cubeProgram, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glUseProgram(modelReflect);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		{
			time = glfwGetTime();
			feature1 = true;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		{
			time1 = glfwGetTime();
			feature2 = true;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			time2 = glfwGetTime();
			feature3 = true;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		{
			time3 = glfwGetTime();
			feature4 = true;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		{
			time4 = glfwGetTime();
			feature5 = true;
		}

		glm::mat4 trans(1);
		trans = glm::translate(trans, glm::vec3(0, 0, 0));
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "trans"), 1, GL_FALSE, glm::value_ptr(trans)); 
		if (glfwGetTime() - time<36/8)
			model = glm::translate(model, glm::vec3(0.0f, -1.0f, time*8-glfwGetTime()*8));
		else model = glm::translate(model, glm::vec3(0.0f, -1.0f, -36));
		model = glm::scale(model, glm::vec3(0.2));
		model = glm::rotate(model, (float)glm::radians(180.0f), glm::vec3(0, 1, 0));
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if(feature1)
		ourModel.Draw(modelReflect);


		if(glfwGetTime() - time1<3)
		model = glm::translate(model, glm::vec3(-7+ (glfwGetTime() - time1-2)*14,0, -60+ (glfwGetTime() - time1 - 2)*60));
		else model = glm::translate(model, glm::vec3(7.0f, 0, 0));
		glBindTexture(GL_TEXTURE_CUBE_MAP, man3);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if(feature2)
		ourModel.Draw(modelReflect);


		model=glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, -36));
		model = glm::scale(model, glm::vec3(0.2));
		model = glm::rotate(model, (float)glm::radians(180.0f), glm::vec3(0, 1, 0));
		if (glfwGetTime() - time2<4)
			model = glm::translate(model, glm::vec3(7- (glfwGetTime() - time2 - 2) * 7, 0, -60 + (glfwGetTime() - time2 - 2) * 30));
		else model = glm::translate(model, glm::vec3(-7.0f, 0, 0));
		glBindTexture(GL_TEXTURE_CUBE_MAP, man2);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if (feature3)
		ourModel.Draw(modelReflect);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, -36));
		model = glm::scale(model, glm::vec3(0.2));
		model = glm::rotate(model, (float)glm::radians(180.0f), glm::vec3(0, 1, 0));
		if (glfwGetTime() - time3<2.5)
			model = glm::translate(model, glm::vec3(-14+(glfwGetTime() - time3 - 1.5) * 14*2, 0, -60 + (glfwGetTime() - time3 - 1.5) *60));
		else model = glm::translate(model, glm::vec3(14.0f, 0, 0));
		glBindTexture(GL_TEXTURE_CUBE_MAP, man1);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if (feature4)
		ourModel.Draw(modelReflect);


		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, -36));
		model = glm::scale(model, glm::vec3(0.2));
		model = glm::rotate(model, (float)glm::radians(180.0f), glm::vec3(0, 1, 0));
		if (glfwGetTime() - time4<3.5)
			model = glm::translate(model, glm::vec3(14 -(glfwGetTime() - time4 - 1.5) * 14, 0, -60 + (glfwGetTime() - time4 - 1.5) * 30));
		else model = glm::translate(model, glm::vec3(-14.0f, 0, 0));
		glBindTexture(GL_TEXTURE_CUBE_MAP, man4);
		glUniformMatrix4fv(glGetUniformLocation(modelReflect, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if (feature5)
		ourModel.Draw(modelReflect);

		model= glm::mat4(1);

		glUseProgram(shaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
			
		glUseProgram(lightingProgram);
		glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

		glUseProgram(lampProgram);
		glUniformMatrix4fv(glGetUniformLocation(lampProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lampProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
		
		glBindVertexArray(insideVAO);
		for (unsigned int i = 0; i < 27; i++)
			for (unsigned int j = 0; j < 18; j++)
			{
				float r= (float)sin((glfwGetTime()/5*i)* 0.9f) / 2 + 0.5;
				float g = (float)sin((glfwGetTime() / 5 *i)* 1.4f) / 2 + 0.5;
				float b = (float)sin((glfwGetTime() / 5 *i)* 0.5f) / 2 + 0.5;
				glUniform4f(glGetUniformLocation(lampProgram, "color"), r,g,b, 1.0f);
				model = glm::mat4(1);
				model = glm::translate(model, glm::vec3(-7.59, 0.55f*j-0.68, i*0.7-49));
				model = glm::scale(model, glm::vec3(0.24f));
				float angle = 20.0f * j*sin(i/**sin(i)*/);
				model = glm::rotate(model, glm::radians(angle) + (float)glfwGetTime(), glm::vec3(1,0,0));
				glUniformMatrix4fv(glGetUniformLocation(lampProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				model = glm::translate(model, glm::vec3(83.3-0.02, 0, 0));
				glUniformMatrix4fv(glGetUniformLocation(lampProgram, "model"), 1, GL_FALSE, &model[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		model = glm::mat4(1);
		glUseProgram(shaderProgram);
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		for (int i = 0; i < 4; i++) {
			colorvertices[3 + i * 6] =(float)sin((glfwGetTime()*i)* 0.9f)/2+0.5;
			colorvertices[4 + i * 6] = (float)sin((glfwGetTime()*i)* 1.4f) / 2 + 0.5;
			colorvertices[5 + i * 6] = (float)sin((glfwGetTime()*i)* 0.5f) / 2 + 0.5;

			colortop[3 + i * 6] = (float)sin((glfwGetTime()*i)* 0.9f) / 2 + 0.5;
			colortop[4 + i * 6] = (float)sin((glfwGetTime()*i)* 1.4f) / 2 + 0.5;
			colortop[5 + i * 6] = (float)sin((glfwGetTime()*i)* 0.5f) / 2 + 0.5;
		}

		glUseProgram(colorProgram);
		glUniformMatrix4fv(glGetUniformLocation(colorProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(colorProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
		glGenVertexArrays(1, &colorVAO);
		glGenBuffers(1, &colorVBO);
		glGenBuffers(1, &ebo);
		glBindVertexArray(colorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorvertices), colorvertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glGenVertexArrays(1, &colorTopVAO);
		glGenBuffers(1, &colorTopVBO);
		glGenBuffers(1, &ebo);
		glBindVertexArray(colorTopVAO);
		glBindBuffer(GL_ARRAY_BUFFER, colorTopVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colortop), colortop, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		std::cin.clear();
		//std::cin.ignore(100, GLFW_KEY_SPACE);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			process = !process;
		if (process)
		{
			if (film == 34)
				film = 2;
			else
				film += 0.5;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			filmStartTime = glfwGetTime();
			playfilm = true;
		}

		unsigned int frame = f1;
		if ((int)film == 0) frame = f1;
		if ((int)film == 1)  frame =f2;
		if ((int)film == 2) frame =f3;
		if ((int)film == 3) frame = f4;
		if ((int)film == 4) frame = f5;
		if ((int)film == 5) frame = f6;
		if ((int)film == 6)  frame = f7;
		if ((int)film == 7) frame = f8;
		if ((int)film == 8) frame = f9;
		if ((int)film == 9) frame = f10;
		if ((int)film == 10) frame = f11;
		if ((int)film == 11)  frame = f12;
		if ((int)film == 12) frame = f13;
		if ((int)film == 13) frame = f14;
		if ((int)film ==14) frame = f15;
		if ((int)film == 15) frame = f16;
		if ((int)film == 16)  frame = f17;
		if ((int)film == 17) frame = f18;
		if ((int)film == 18) frame = f19;
		if ((int)film == 19) frame = f20;
		if ((int)film == 20) frame = f21;
		if ((int)film == 21)  frame = f22;
		if ((int)film == 22) frame = f23;
		if ((int)film == 23) frame = f24;
		if ((int)film == 24) frame = f25;
		if ((int)film == 25) frame = f26;
		if ((int)film == 26)  frame = f27;
		if ((int)film == 27) frame = f28;
		if ((int)film == 28) frame = f29;
		if ((int)film == 29) frame = f30;
		if ((int)film == 30) frame = f31;
		if ((int)film == 31)  frame = f32;
		if ((int)film == 32) frame = f33;
		if ((int)film == 33) frame = f34;

			model = glm::mat4(1);	
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！7
			model = glm::translate(model, glm::vec3(20, 0, -40));
			glUseProgram(shaderProgram);
			glBindVertexArray(transparentVAO);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！1
			model = glm::translate(model, glm::vec3(0.01, 0, 0));
			glUseProgram(colorProgram);
			glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(colorVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！3
			model = glm::translate(model, glm::vec3(-20.01, 0, 0));
			model = glm::rotate(model, (float)glm::radians(90.0f), glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3(0, 0, -20));
			model = glm::rotate(model, (float)glm::radians(90.0f), glm::vec3(0, 1, 0));
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(transparentVAO);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！2
			model = glm::translate(model, glm::vec3(0.01, 0, 0));
			glUseProgram(colorProgram);
			glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(colorVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！10
			if(10 - (glfwGetTime() - filmStartTime) * 3>0)
			model = glm::translate(model, glm::vec3(-20.01, (glfwGetTime()-filmStartTime)*3, 20));
			else model = glm::translate(model, glm::vec3(-20.01, 0, 20));
			model = glm::rotate(model, (float)glm::radians(-90.0f), glm::vec3(0, 1, 0));
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(mubuVAO);
			if (10 - (glfwGetTime() - filmStartTime) * 3 > 0||!playfilm) {
				glBindTexture(GL_TEXTURE_2D, transparentTexture);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！11
			if (10 - (glfwGetTime() - filmStartTime) * 3>0)
				model = glm::translate(model, glm::vec3(0.005, -(glfwGetTime() - filmStartTime) * 3, 0));
			else model = glm::translate(model, glm::vec3(0.005, 0, 0));
			glBindVertexArray(transparentVAO);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindTexture(GL_TEXTURE_2D, frame);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			model = glm::translate(model, glm::vec3(0.005, 0, 0));
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！4
			glUseProgram(colorProgram);
			glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(colorVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！6
			model = glm::translate(model, glm::vec3(-20, 0, 0));
			glUseProgram(colorProgram);
			glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(colorVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			model = glm::translate(model, glm::vec3( -0.01, 0, 0));
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！15		
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(transparentVAO);
			glBindTexture(GL_TEXTURE_2D, cinema);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！15		
			glUseProgram(shaderProgram);
			model = glm::translate(model, glm::vec3(0.015, 0, 0));
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(transparentVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			model = glm::mat4(1);
			model = glm::rotate(model, (float)glm::radians(-180.0f), glm::vec3(0, 0, 1));
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！18
			model = glm::translate(model, glm::vec3(0, -8.03, -40));
			glUseProgram(cubeProgram);
			glBindVertexArray(cubeVAO);
			glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindTexture(GL_TEXTURE_2D, floor);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！13
			model = glm::mat4(1);
			model = glm::translate(model, glm::vec3(0, -0.02, -40));
			glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindTexture(GL_TEXTURE_2D, cell);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！billboard
			model = glm::translate(model, glm::vec3(0, 11.03, -20));
			glUseProgram(lampProgram); 
			float r1 = (float)sin((glfwGetTime() / 2)* 0.9f) / 2 + 0.7;
			float g1 = (float)sin((glfwGetTime() / 2)* 1.4f) / 2 + 0.7;
			float b1 = (float)sin((glfwGetTime() / 2)* 0.5f) / 2 + 0.7;
			glUniform4f(glGetUniformLocation(lampProgram, "color"), r1, g1, b1, 1.0f);
			model = glm::translate(model, glm::vec3(0,-10, 29.995));
			glBindVertexArray(billboardVAO);
			glUniformMatrix4fv(glGetUniformLocation(lampProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			model = glm::translate(model, glm::vec3(0, -1, -9.995));
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！5
			glUseProgram(colorProgram);
			glUniformMatrix4fv(glGetUniformLocation(colorProgram, "model"), 1, GL_FALSE, &model[0][0]);
			glBindVertexArray(colorTopVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glDepthFunc(GL_LEQUAL);
		glUseProgram(lightingProgram); 
		view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
		glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lightingProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}