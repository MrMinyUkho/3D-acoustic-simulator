//
// Created by adima on 18.07.2024.
//



#include "../defines.h"

char *ShaderLoadFile(char *FileName) {
    FILE *f;
    errno_t err = fopen_s(&f, FileName, "rb");
    if(err != 0) printf("Error(%d) with load %s shader\n", err, FileName);
    else printf("Succes loaded %s shader\n", FileName);

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    char *text = malloc(len + 1);
    memset(text, 0, len + 1);

    fseek(f, 0, SEEK_SET);
    fread(text, 1, len, f);
    fclose(f);

    return text;
}

GLuint ShaderCompile(char* FileName, GLuint type) {
    char *txt = ShaderLoadFile(FileName);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &txt, NULL);
    glCompileShader(shader);


    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        GLchar log[2000];
        glGetShaderInfoLog(shader, 2000, NULL, log);
        printf("%s\n", log);
    }

    free(txt);

    return shader;
}

GLuint ShaderCreateProgram(char *VertexFileName, char *FragmentFileName) {
    const GLuint frag = ShaderCompile(FragmentFileName, GL_FRAGMENT_SHADER);
    const GLuint vert = ShaderCompile(VertexFileName, GL_VERTEX_SHADER);

    const GLuint prog = glCreateProgram();
    glAttachShader(prog, frag);
    glAttachShader(prog, vert);
    glLinkProgram(prog);

    GLint ok;
    glGetProgramiv(prog, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        GLchar log[2000];
        glGetProgramInfoLog(prog, 2000, NULL, log);
        printf("%s\n", log);
    }

    return prog;
}