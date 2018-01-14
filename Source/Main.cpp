#include <iostream>
#include <string>
#include <vector>

//For sleep
#include <chrono>
#include <thread>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Maze.h"

Maze maze;

sf::Window *window;

glm::mat4 aspectMat;
glm::mat4 orthoMat;

GLuint shaderProgram;
GLuint orthoPos;
GLuint aspectPos;
//VAO
GLuint vao;
//VBOs
GLuint vertexBuffer;
GLuint colorBuffer;

GLuint initShaders(const std::string &vertexPath, const std::string &fragmentPath)
{
	GLuint vertexID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentID = glCreateShader(GL_FRAGMENT_SHADER);

	//Load vertex shader code from file
	std::string vertexCode;
	std::ifstream file(vertexPath);
	if (!file.is_open())
	{
		std::cout << "Failed to open vertex shader from " + vertexPath << std::endl;
		std::cin.get();
		exit(1);
	}
	
	std::string line;
	while (std::getline(file, line))
	{
		vertexCode += line;
		vertexCode += '\n';
	}
	file.close();

	//Load fragment shader code from file
	std::string fragmentCode;
	file.open(fragmentPath);
	if (!file.is_open())
	{
		std::cout << "Failed to open fragment shader from " + fragmentPath << std::endl;
		std::cin.get();
		exit(1);
	}

	while (std::getline(file, line))
	{
		fragmentCode += line;
		fragmentCode += '\n';
	}

	//Compile vertex
	const char *vertexConst = vertexCode.c_str();
	glShaderSource(vertexID, 1, &vertexConst, NULL);
	glCompileShader(vertexID);

	//Check vertex
	GLint result = GL_FALSE;
	glGetShaderiv(vertexID, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int logLength;
		glGetShaderiv(vertexID, GL_INFO_LOG_LENGTH, &logLength);

		char *infoLog = (char*)malloc(logLength + 1);
		glGetShaderInfoLog(vertexID, logLength, NULL, infoLog);

		std::cout << infoLog << std::endl;
		std::cin.get();
		exit(1);
	}
	
	//Compile fragment
	const char *fragmentConst = fragmentCode.c_str();
	glShaderSource(fragmentID, 1, &fragmentConst, NULL);
	glCompileShader(fragmentID);

	//Check fragment
	result = GL_FALSE;
	glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int logLength;
		glGetShaderiv(fragmentID, GL_INFO_LOG_LENGTH, &logLength);

		char *infoLog = (char*)malloc(logLength + 1);
		glGetShaderInfoLog(fragmentID, logLength, NULL, infoLog);

		std::cout << infoLog << std::endl;
		std::cin.get();
		exit(1);
	}

	//Link the program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);

	//Check the program
	result = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	if (result == GL_FALSE)
	{
		int logLength;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);

		char *infoLog = (char*)malloc(logLength + 1);
		glGetProgramInfoLog(programID, logLength, NULL, infoLog);

		std::cout << infoLog << std::endl;
		std::cin.get();
		exit(1);
	}

	//Cleanup
	glDetachShader(programID, vertexID);
	glDetachShader(programID, fragmentID);

	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);

	return programID;
}

glm::vec3 getColor(Node::Type type)
{
	switch (type)
	{
	case Node::Type::SEARCHED:
		return glm::vec3(0.0, 0.4, 0.0);

	case Node::Type::START:
		return glm::vec3(0.0, 1.0, 0.25);

	case Node::Type::TARGET:
		return glm::vec3(1.0, 0.65, 0.0);

	case Node::Type::TRACED:
		return glm::vec3(0.1, 0.6, 1.0);

	case Node::Type::UNSEARCHED:
		return glm::vec3(1.0, 1.0, 1.0);

	case Node::Type::WALL:
		return glm::vec3(0.2, 0.2, 0.2);
	}
}

void initGraphics()
{
	window = new sf::Window(sf::VideoMode(1920, 1080), "Maze");
	window->setActive(true);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to init GLEW." << std::endl;
		std::cin.get();
		exit(1);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	shaderProgram = initShaders("Shaders\\vertex.gsl", "Shaders\\fragment.gsl");

	int xCount = maze.getX();
	int yCount = maze.getY();

	double windowAspect = (double)window->getSize().x / (double)window->getSize().y;
	double targetAspect = (double)xCount / (double)yCount;

	//Default aspect ratio (if window and maze have same ratio)
	aspectMat = glm::ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	//Window is wider
	//Touches vertical edges
	if (windowAspect > targetAspect)
	{
		aspectMat = glm::ortho(-windowAspect / targetAspect, windowAspect / targetAspect, -1.0, 1.0, -1.0, 1.0);
	}
	//Window is taller
	//Touches horizontal edges
	else if (windowAspect < targetAspect)
	{
		aspectMat = glm::ortho(-1.0, 1.0, -targetAspect / windowAspect, targetAspect / windowAspect, -1.0, 1.0);
	}

	aspectPos = glGetUniformLocation(shaderProgram, "aspect");

	orthoMat = glm::ortho(0.0, (double)xCount, (double)yCount, 0.0, -1.0, 1.0);
	orthoPos = glGetUniformLocation(shaderProgram, "ortho");

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &colorBuffer);

	std::vector<glm::vec2> vertexData;
	//Fill vertex buffer because it won't change
	for (int i = 0; i < xCount; i++)
	{
		for (int o = 0; o < yCount; o++)
		{
			vertexData.push_back(glm::vec2(i, o));
			vertexData.push_back(glm::vec2(i + 1, o));
			vertexData.push_back(glm::vec2(i, o + 1));

			vertexData.push_back(glm::vec2(i + 1, o));
			vertexData.push_back(glm::vec2(i, o + 1));
			vertexData.push_back(glm::vec2(i + 1, o + 1));
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(vertexData[0]), &vertexData[0], GL_STATIC_DRAW);
	std::vector<glm::vec3> colorData;
	//Fill buffers
	for (int i = 0; i < maze.getX(); i++)
	{
		for (int o = 0; o < maze.getY(); o++)
		{
			glm::vec3 color = getColor(maze.getType(i, o));

			colorData.push_back(color);
			colorData.push_back(color);
			colorData.push_back(color);
			colorData.push_back(color);
			colorData.push_back(color);
			colorData.push_back(color);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(colorData[0]), &colorData[0], GL_DYNAMIC_DRAW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(aspectPos, 1, GL_FALSE, &aspectMat[0][0]);
	glUniformMatrix4fv(orthoPos, 1, GL_FALSE, &orthoMat[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, maze.getX() * maze.getY() * 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	window->display();
}

void updateData(glm::vec2 position)
{
	int x = position.x;
	int y = position.y;

	std::vector<glm::vec3> colorData;
	glm::vec3 color = getColor(maze.getType(x, y));

	colorData.push_back(color);
	colorData.push_back(color);
	colorData.push_back(color);
	colorData.push_back(color);
	colorData.push_back(color);
	colorData.push_back(color);

	int offset = 6 * (x * maze.getY() + y);
	offset *= sizeof(colorData[0]);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, offset, colorData.size() * sizeof(colorData[0]), &colorData[0]);
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(aspectPos, 1, GL_FALSE, &aspectMat[0][0]);
	glUniformMatrix4fv(orthoPos, 1, GL_FALSE, &orthoMat[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, maze.getX() * maze.getY() * 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	window->display();
}

void drawProcess()
{
	int sleepMS = 15;
	if (maze.getX() * maze.getY() > 625) sleepMS = 5;
	if (maze.getX() * maze.getY() > 2500) sleepMS = 2;

	glm::vec2 updatePos;
	while (maze.stepProcess(updatePos))
	{
		//Looks cool for visualizations, but is shortend for large mazes
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepMS));

		updateData(updatePos);
		draw();
	}

	std::cout << "Solution found in " << maze.getSteps() << " steps." << std::endl;
}

void drawTrace()
{
	int traceMS = 5000;
	int steps = maze.getSteps();

	glm::vec2 updatePos;
	while (maze.stepTrace(updatePos))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(traceMS / steps));

		updateData(updatePos);

		draw();
	}
}

int main()
{
	maze = Maze();
	//maze.load("mazes\\maze3.txt");
	maze.generate(750, 500);

	initGraphics();
	drawProcess();
	drawTrace();

	std::cout << "Tracing complete." << std::endl;

	//drawProcess();
	//drawTrace();

	/*
	std::cout << "Would you like to [1]load and test a maze, or [2]generate a new maze?" << std::endl;
	char selection = std::cin.get();

	if (selection == '1')
	{
		std::string path;
		std::cout << "Enter the filepath of the maze." << std::endl;
		std::cin >> path;

		loadMaze(maze, path);
		processMaze(maze);
		traceMaze(maze);
	}
	else if (selection == '2')
	{
		generateMaze(maze);
		processMaze(maze);
		traceMaze(maze);
	}
	else
	{
		std::cout << "Invalid input, exiting program." << std::endl;
		exit(0);
	}
	*/

	std::cin.get();

	return 0;
}