#version 430 core
layout (location = 0) in vec3 aPos;

layout (location = 1) in vec2 aTexCoord;
uniform vec3 iResolution;
uniform mat4 model,view,projection;
out vec2 fragCoord;
out vec2 texCoord;
out vec3 vecColor;
void main()
{
    fragCoord=vec2((aPos.x+0.5)*iResolution.x, (0.5+aPos.y)*iResolution.y);
    gl_Position= projection*view*model*vec4(aPos,1.0f);

    texCoord=aTexCoord;
    //vecColor=aColor;
}