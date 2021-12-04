#include "Scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

extern int windowWidth, windowHeight;
extern Camera gCamera;

Scene::Scene()
{
	spotLights.emplace_back(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(2.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), 200, glm::radians(30.0f), glm::radians(45.0f));

	glGenBuffers(1, &projViewUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, projViewUBO);

	glGenBuffers(1, &lightsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight) + sizeof(SpotLight) + sizeof(DirectionalLight), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightsUBO);

	GLuint shaderBlockIndex = glGetUniformBlockIndex(shader.id, "Matrices");
	glUniformBlockBinding(shader.id, shaderBlockIndex, 0);
	GLuint shaderLightsBlockIndex = glGetUniformBlockIndex(shader.id, "Lights");
	glUniformBlockBinding(shader.id, shaderLightsBlockIndex, 1);

	animatedModels.emplace_back("Models/Warrok/sitting_laughing.fbx");
}

void Scene::Draw(float dt, float time)
{
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, glm::vec3(0, -2, 0));
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	glm::mat4 view = gCamera.GetViewMatrix();
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view) * glm::mat3(model)));
	glm::mat4 proj = glm::perspective(glm::radians(gCamera.Zoom), float(windowWidth) / float(windowHeight), 0.001f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	SpotLight viewSpaceLight = spotLights[0];
	viewSpaceLight.position = view * glm::vec4(viewSpaceLight.position, 1.0f);
	viewSpaceLight.direction = view * glm::vec4(viewSpaceLight.direction, 0.0f);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(PointLight), sizeof(SpotLight), &viewSpaceLight);

	shader.use();
	shader.SetMat4("model", glm::value_ptr(model));
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	animatedModels[0].Draw(shader, dt);
}
