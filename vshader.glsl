#version 410 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 projection;  // This is actually view-projection combined

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    // Transform position to world space
    vec4 worldPos = model * vPosition;
    
    // Pass world position to fragment shader for lighting calculations
    FragPos = worldPos.xyz;
    
    // Transform normal to world space (should use normal matrix for non-uniform scaling)
    Normal = mat3(transpose(inverse(model))) * vNormal;
    
    // Pass texture coordinates through
    TexCoord = vTexCoord;

    // Transform to clip space using combined view-projection matrix
    gl_Position = projection * worldPos;
}