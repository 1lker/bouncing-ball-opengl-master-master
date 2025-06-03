#version 410 core

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 projection;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec4 objColor;
uniform float shininess;
uniform float specularStrength;

uniform bool useAmbient;
uniform bool useDiffuse;
uniform bool useSpecular;

out vec3 lightingResult;
out vec2 fTexCoord;

void main() {
    vec3 FragPos = vec3(model * vPosition);
    vec3 norm = normalize(mat3(transpose(inverse(model))) * vNormal);
    vec3 lightDirection = normalize(-lightDir);
    vec3 viewDirection = normalize(viewPos - FragPos);

    vec3 ambient = useAmbient ? 0.2 * vec3(objColor) : vec3(0.0);

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = useDiffuse ? diff * vec3(objColor) : vec3(0.0);

    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    vec3 specular = useSpecular ? specularStrength * spec * vec3(1.0) : vec3(0.0);

    lightingResult = ambient + diffuse + specular;
    fTexCoord = vTexCoord;
    gl_Position = projection * model * vPosition;
}
