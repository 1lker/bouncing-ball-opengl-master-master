#version 410 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;          // will reach the fragment shader

void main()
{
    vec4 worldPos = model * vPosition;

    FragPos  = worldPos.xyz;
    Normal   = mat3(transpose(inverse(model))) * vNormal;
    TexCoord = vTexCoord;

    gl_Position = projection * worldPos;
}
