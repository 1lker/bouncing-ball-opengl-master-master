#version 410 core

in vec3 lightingResult;
in vec2 fTexCoord;

uniform sampler2D textureMap;
uniform bool useTexture;
uniform vec4 objColor;

out vec4 fColor;

void main() {
    vec3 color = lightingResult;
    if (useTexture)
        color *= texture(textureMap, fTexCoord).rgb;
    fColor = vec4(color, objColor.a);
}
