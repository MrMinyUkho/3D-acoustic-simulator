#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in float amplitude;

out vec4 color;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    float a = amplitude/600;
    if(a > 0) color = vec4(0.2, 0.9, 0.3, a);
    else color = vec4(0.8, 0.1, 0.7, -a);
}