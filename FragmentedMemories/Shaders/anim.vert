#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in uint aJointIndices;
layout(location = 4) in vec4 aJointWeights;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform mat4 skinning_matrices[100];

uniform mat4 model;

out vec2 texCoords;

void main()
{
    vec4 modelSpacePos = vec4(aPos, 1.0);
    mat4 modelSpaceMatrix = skinning_matrices[aJointIndices & 0x00000FFu] * aJointWeights.x;
    modelSpaceMatrix += skinning_matrices[(aJointIndices >> 8) & 0x00000FFu] * aJointWeights.y;
    modelSpaceMatrix += skinning_matrices[(aJointIndices >> 16) & 0x00000FFu] * aJointWeights.z;
    modelSpaceMatrix += skinning_matrices[aJointIndices >> 24] * aJointWeights.w;
    modelSpacePos = modelSpaceMatrix * modelSpacePos;

    texCoords = aTexCoords;
    gl_Position = projection * view * model * modelSpacePos;
}