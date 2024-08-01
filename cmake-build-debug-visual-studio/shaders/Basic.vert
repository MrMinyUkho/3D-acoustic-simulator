#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 ViewPos;
out vec3 Normal;

void main() {
    ViewPos = vec3(view[3][0], view[3][1],view[3][2]);
    FragPos = vec3(transform * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(transform))) * inNormal; // корректное преобразование нормалей
    gl_Position = projection * view * vec4(FragPos, 1.0);
}