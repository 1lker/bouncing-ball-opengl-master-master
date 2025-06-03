#version 410 core

in  vec3 FragPos;
in  vec3 Normal;
in  vec2 TexCoord;          

out vec4 fColor;

/* uniforms */
uniform vec4  objColor;
uniform vec3  lightDir;
uniform vec3  viewPos;
uniform sampler2D textureMap;

uniform bool  useAmbient, useDiffuse, useSpecular;
uniform float shininess;
uniform float specularStrength;

void main()
{
    /* --- lighting ---------------------------------------------------- */
    vec3 n  = normalize(gl_FrontFacing ? Normal : -Normal);
    vec3 L  = normalize(-lightDir);
    vec3 V  = normalize(viewPos - FragPos);
    vec3 R  = reflect(-L, n);

    float diff = max(dot(n, L), 0.0);

    vec3 ambient  = useAmbient  ? 0.5 * objColor.rgb   : vec3(0.0);
    vec3 diffuse  = useDiffuse  ? diff * objColor.rgb  : vec3(0.0);
    vec3 specular = vec3(0.0);
    if (useSpecular && diff > 0.0)
        specular = specularStrength * pow(max(dot(R, V), 0.0), shininess) * vec3(1.0);

    /* --- texture ----------------------------------------------------- */
    vec3 tex = texture(textureMap, TexCoord).rgb;

    fColor = vec4( (ambient + diffuse + specular) * tex, objColor.a );
}
