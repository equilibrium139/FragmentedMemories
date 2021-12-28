#version 330 core

out vec4 color;

uniform sampler2D ourTexture;

in vec2 texCoords;

void main()
{
    color = vec4(1, 0, 0, 1);
}