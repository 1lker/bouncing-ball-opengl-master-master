#version 410 core

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 projection;  // This is actually view-projection combined
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
    // Transform position to world space
    vec4 worldPos = model * vPosition;
    vec3 FragPos = worldPos.xyz;
    
    // Transform normal to world space
    vec3 norm = normalize(mat3(transpose(inverse(model))) * vNormal);
    
    // Lighting calculations in world space
    vec3 lightDirection = normalize(-lightDir);
    vec3 viewDirection = normalize(viewPos - FragPos);

    // Ambient component
    vec3 ambient = useAmbient ? 0.2 * vec3(objColor) : vec3(0.0);

    // Diffuse component
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = useDiffuse ? diff * vec3(objColor) : vec3(0.0);

    // Specular component
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    vec3 specular = useSpecular ? specularStrength * spec * vec3(1.0) : vec3(0.0);

    // Combine all lighting components
    lightingResult = ambient + diffuse + specular;
    
    // Pass texture coordinates through
    fTexCoord = vTexCoord;
    
    // Transform to clip space using combined view-projection matrix
    gl_Position = projection * worldPos;
}