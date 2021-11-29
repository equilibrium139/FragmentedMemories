#version 330 core

out vec4 color;

uniform sampler2D diffuse;
uniform sampler2D specular;

in vec2 texCoords;

void main()
{
    color = texture(diffuse, texCoords);
    // color = vec4(1.0, 0.0, 0.0, 1.0);
}