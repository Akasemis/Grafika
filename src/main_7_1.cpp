#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;

//vector ryb 
glm::vec3 fishVector[10];
glm::vec3 fishVec[10];

Core::Shader_Loader shaderLoader;
obj::Model groundModel;
obj::Model sharkModel;
obj::Model fishModel;
glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; 
glm::vec3 cameraSide; 
float cameraAngle = 0;
float cameraRotation = 0;
float MouseLastPos[2] = { 0,0 };
float MouseDif[2] = { 0,0 };

bool cameraFollow = true;


glm::mat4 cameraMatrix, perspectiveMatrix;
glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));
glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureFish;
GLuint textureGround;

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;

	if (cameraFollow) {
		switch (key)
		{
		case 'z': cameraAngle -= angleSpeed; break;
		case 'x': cameraAngle += angleSpeed; break;
		case 'w': cameraPos += cameraDir * moveSpeed; break;
		case 's': cameraPos -= cameraDir * moveSpeed; break;
		case 'd': cameraPos += cameraSide * moveSpeed; break;
		case 'a': cameraPos -= cameraSide * moveSpeed; break;
		case 'v': cameraFollow = false; cameraPos = glm::vec3(0, 0, 16); rotation = glm::quat(1, 0, 0, 0); break;
		}
	}
	else {
		switch (key)
		{
		case 'd': cameraRotation -= 1.0f; break;
		case 'a': cameraRotation += 1.0f; break;
		case 'v': cameraFollow = true; break;
		}
	}
}

void mouse(int x, int y)
{
	float mousePos[2] = { x, y };
	MouseDif[0] = MouseLastPos[0] - mousePos[0];
	MouseDif[1] = MouseLastPos[1] - mousePos[1];
	MouseLastPos[0] = mousePos[0];
	MouseLastPos[1] = mousePos[1];
}

glm::mat4 createCameraMatrix()
{
	if (cameraFollow) {
		glm::quat obrotY = glm::angleAxis(MouseDif[0] * 0.02f, glm::vec3(0, 1, 0));
		glm::quat obrotX = glm::angleAxis(MouseDif[1] * 0.02f, glm::vec3(1, 0, 0));

		glm::quat rotationChange = obrotX * obrotY;

		rotation = rotationChange * rotation;
		rotation = glm::normalize(rotation);

		MouseDif[0] = 0;
		MouseDif[1] = 0;

		cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
		cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);
	}
	return Core::createViewMatrixQuat(cameraPos, rotation);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void renderScene()
{

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	cameraMatrix = createCameraMatrix();
	if (!cameraFollow)
		cameraMatrix *= glm::rotate(glm::radians(cameraRotation), glm::vec3(0, 1, 0));
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.08f, 0.08f, 0.5f, 1.0f);

	glm::mat4 fishRotation = glm::mat4_cast(glm::inverse(rotation));
	glm::mat4 fishInitialTransformation = glm::translate(glm::vec3(0,-0.25f,-1.4)) * glm::rotate(glm::radians(180.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.1f));
	glm::mat4 fishModelMatrix;
	if (cameraFollow)
		fishModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * fishRotation * fishInitialTransformation;
	else
		fishModelMatrix = fishRotation * fishInitialTransformation;
	

	drawObjectColor(&sharkModel, fishModelMatrix, glm::vec3(0.6f));

	glm::mat4 trans1 = {
		0.2,0.0f,0,0,
		0,0.2,0,0,
		0,0,0.2,0,
		time*0.1,time*0.1,time * 1,1
	};
	drawObjectTexture(&groundModel, glm::translate(glm::vec3(0, -5, -3))* glm::rotate(glm::radians(270.0f), glm::vec3(0, 1, 0)), textureGround);

	for (int i = 0; i < 5; i++) {
		drawObjectTexture(&fishModel, glm::translate(fishVector[i]) *glm::rotate(glm::radians(180.0f), glm::vec3(-1, 0, -1))*trans1, textureFish);
	}
	for (int i = 0; i < 10; i++)
	{
		drawObjectTexture(&fishModel, glm::translate(fishVector[i]) *glm::rotate(glm::radians(0.0f), glm::vec3(-1, -1, -1) + glm::vec3(0, -1, 0))*trans1, textureFish);
	}

	for (int i = 0; i < 10; i++)
	{
		drawObjectTexture(&fishModel, glm::translate(fishVector[i]) *glm::rotate(glm::radians(180.0f), fishVec[i])*trans1, textureFish);
	}
	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	

	sharkModel = obj::loadModelFromFile("models/shark.obj");
	fishModel = obj::loadModelFromFile("models/fish.obj");
	groundModel = obj::loadModelFromFile("models/sand.obj");
	
	
	textureFish = Core::LoadTexture("textures/fish.png");
	textureGround = Core::LoadTexture("textures/sand.png");

	for (int i = 0; i < 10; i++) {
		fishVector[i] = glm::ballRand(5.0f);
	}

	for (int i = 0; i < 10; i++) {
		fishVec[i] = glm::ballRand(5.0f);
	}
	


}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
