
#ifndef GL_EXTRA_H_
#define GL_EXTRA_H_

#include <stdbool.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#define FILE_IMPLEMENTATION
#include "file.h"

const char *shader_type_as_cstr(GLuint shader);

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader);
bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader);
bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program);

#ifdef GL_EXTRA_IMPLEMENTATION

const char *shader_type_as_cstr(GLuint shader)
{
    switch (shader) {
    case GL_VERTEX_SHADER:
        return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER:
        return "GL_FRAGMENT_SHADER";
    default:
        return "(Unknown)";
    }
}

bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader)
{
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint compiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
        fprintf(stderr, "[ERROR] could not compile %s\n", shader_type_as_cstr(shader_type));
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }
    return true;
}

bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader)
{
    char *source = slurp_file(file_path);

    bool ok = compile_shader_source(source, shader_type, shader);
    if (!ok) {
        fprintf(stderr, "[ERROR] failed to compile shader file `%s`: %s\n", file_path, strerror(errno)); 
    }
    free(source);
    return ok;
}

bool link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program)
{
    *program = glCreateProgram();
    glAttachShader(*program, vert_shader);
    glAttachShader(*program, frag_shader);
    glLinkProgram(*program);

    GLint linked = 0;
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
        fprintf(stderr, "[ERROR] Program linking %.*s\n", message_size, message);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return program;
}

#endif // GL_EXTRA_IMPLEMENTATION
#endif // GL_EXTRA_H_
