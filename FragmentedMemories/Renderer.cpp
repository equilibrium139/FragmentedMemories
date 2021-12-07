#include "Renderer.h"

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

Renderer::Renderer(unsigned int projViewUBO, unsigned int lightsUBO)
	:projViewUBO(projViewUBO), lightsUBO(lightsUBO)
{
	shader.use();
	GLuint shaderBlockIndex = glGetUniformBlockIndex(shader.id, "Matrices");
	glUniformBlockBinding(shader.id, shaderBlockIndex, 0);
	GLuint shaderLightsBlockIndex = glGetUniformBlockIndex(shader.id, "Lights");
	glUniformBlockBinding(shader.id, shaderLightsBlockIndex, 1);
}

void Renderer::Draw(Scene& scene, float dt, int windowWidth, int windowHeight, int maxPointLights, int maxSpotLights)
{
	assert(scene.pointLights.size() <= maxPointLights);
	assert(scene.spotLights.size() <= maxSpotLights);

	glm::mat4 view = scene.camera.GetViewMatrix();
	glm::mat4 proj = scene.camera.GetProjectionMatrix((float)windowWidth / (float)windowHeight);

	glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
		glm::value_ptr(proj));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
		glm::value_ptr(view));

	/*std::vector<glm::vec3> worldSpaceLightPositions(scene.pointLights.size() + scene.spotLights.size());
	for (int i = 0; i < scene.pointLights.size(); i++)
	{
		auto& lightPosition = scene.pointLights[i].position;
		worldSpaceLightPositions[i] = lightPosition;
		lightPosition = view * glm::vec4(lightPosition, 1.0f);
	}
	for (int i = 0; i < scene.spotLights.size(); i++)
	{
		auto& lightPosition = scene.spotLights[i].position;
		worldSpaceLightPositions[i + scene.pointLights.size()] = lightPosition;
		lightPosition = view * glm::vec4(lightPosition, 1.0f);
	}*/

	DirectionalLight dirLightViewSpace = scene.dirLight;
	dirLightViewSpace.direction = view * glm::vec4(dirLightViewSpace.direction, 0.0f);

	int numLights[2] = { scene.pointLights.size(), scene.spotLights.size() };
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	if (scene.pointLights.size() > 0) glBufferSubData(GL_UNIFORM_BUFFER, 0, scene.pointLights.size() * sizeof(PointLight), scene.pointLights.data());
	if (scene.spotLights.size() > 0) glBufferSubData(GL_UNIFORM_BUFFER, maxPointLights * sizeof(PointLight), scene.spotLights.size() * sizeof(SpotLight), scene.spotLights.data());
	glBufferSubData(GL_UNIFORM_BUFFER, maxPointLights * sizeof(PointLight) + maxSpotLights * sizeof(SpotLight), sizeof(DirectionalLight), &dirLightViewSpace);
	glBufferSubData(GL_UNIFORM_BUFFER, maxPointLights * sizeof(PointLight) + maxSpotLights * sizeof(SpotLight) + sizeof(DirectionalLight), sizeof(numLights), numLights);

	// reset light positions to world space
	/*for (int i = 0; i < scene.pointLights.size(); i++)
	{
		auto& lightPosition = scene.pointLights[i].position;
		lightPosition = worldSpaceLightPositions[i];
	}
	for (int i = 0; i < scene.spotLights.size(); i++)
	{
		auto& lightPosition = scene.spotLights[i].position;
		lightPosition = worldSpaceLightPositions[i + scene.pointLights.size()];
	}*/

	shader.use();
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(view * model));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	for (auto& model : scene.animatedModels)
	{
		model.Draw(shader, dt);
	}
}