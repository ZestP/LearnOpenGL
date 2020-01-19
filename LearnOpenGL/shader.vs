#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform vec3 iResolution;
out vec2 fragCoord;
out vec2 texCoord;
void main()
{
    fragCoord=vec2((aPos.x+0.5)*iResolution.x, (0.5+aPos.y)*iResolution.y);
    gl_Position = vec4(2*aPos.x, 2*aPos.y, aPos.z, 1.0);
    texCoord=aTexCoord;
}