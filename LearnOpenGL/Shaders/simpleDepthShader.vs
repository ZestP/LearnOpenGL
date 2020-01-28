#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 lightSpaceMatrixLocation;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrixLocation * model * vec4(position, 1.0f);
}