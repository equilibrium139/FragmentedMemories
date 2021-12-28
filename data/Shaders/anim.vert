#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in uint aJointIndices;
layout(location = 5) in vec4 aJointWeights;
// layout(location = 6) in vec3 aBitangent;

layout (std140) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform mat4 skinning_matrices[100];

uniform mat4 model;
uniform mat3 normalMatrix;

out VS_OUT {
    mat3 TBN;
    vec3 fragViewPos;
    vec2 texCoords;
} vs_out;

void main()
{
    vec4 modelSpacePos = vec4(aPos, 1.0);
    mat4 modelSpaceMatrix = skinning_matrices[aJointIndices & 0xFFu] * aJointWeights.x;
    modelSpaceMatrix += skinning_matrices[(aJointIndices >> 8) & 0xFFu] * aJointWeights.y;
    modelSpaceMatrix += skinning_matrices[(aJointIndices >> 16) & 0xFFu] * aJointWeights.z;
    modelSpaceMatrix += skinning_matrices[(aJointIndices >> 24) & 0xFFu] * aJointWeights.w;
    // mat4 modelSpaceMatrix = identity * aJointWeights.x;
    // modelSpaceMatrix += identity * aJointWeights.y;
    // modelSpaceMatrix += identity * aJointWeights.z;
    // modelSpaceMatrix += identity * aJointWeights.w;
    modelSpacePos = modelSpaceMatrix * modelSpacePos;
    
    mat3 modelSpaceNormalMatrix = transpose(inverse(mat3(modelSpaceMatrix)));
    mat3 finalNormalMatrix = normalMatrix * modelSpaceNormalMatrix;

    // mat3 finalNormalMatrix = transpose(inverse(mat3(view) * mat3(model) * modelSpaceNormalMatrix));

    vec3 normal = finalNormalMatrix * aNormal;
    vec3 tangent = finalNormalMatrix * aTangent;
    vec3 bitangent = finalNormalMatrix * normalize(cross(aNormal, aTangent));

    vec4 viewSpacePos = view * model * modelSpacePos;

    vs_out.TBN = mat3(tangent, bitangent, normal);
    vs_out.fragViewPos = vec3(viewSpacePos);
    vs_out.texCoords = aTexCoords;

    gl_Position = projection * viewSpacePos;
}