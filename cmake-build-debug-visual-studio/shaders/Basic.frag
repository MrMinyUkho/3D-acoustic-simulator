#version 460

layout(location = 0) out vec4 FragColor;

in vec3 ViewPos;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;

void main() {
    vec3 lightPos = vec3(-1.0, 2.0, -2.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 viewPos = vec3(0.0, 2.0, -5.0);

    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
}