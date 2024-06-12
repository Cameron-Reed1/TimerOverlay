#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


#include "shader_utils.h"


GLuint loadShader(const char* const file_path, GLenum shader_type)
{
    GLuint shaderID = glCreateShader(shader_type);

    std::string shaderSrc;
    std::ifstream shaderStream(file_path, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderSrc = sstr.str();

        shaderStream.close();
    } else {
        std::cerr << "Failed to open shader file " << file_path << std::endl;
        return 0;
    }

    std::cout << "Compiling shader " << file_path << std::endl;
    const char* const p_shaderSrc = shaderSrc.c_str();
    glShaderSource(shaderID, 1, &p_shaderSrc, NULL);
    glCompileShader(shaderID);

    GLint result = GL_FALSE;
    int infoLogLen;

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char* shaderErrMsg = new char[infoLogLen + 1];
        glGetShaderInfoLog(shaderID, infoLogLen, NULL, shaderErrMsg);
        std::cerr << shaderErrMsg << std::endl;
    }

    return shaderID;
}

GLuint loadShaderDirect(const char* const shader_src, GLenum shader_type)
{
    GLuint shaderID = glCreateShader(shader_type);
    glShaderSource(shaderID, 1, &shader_src, NULL);
    glCompileShader(shaderID);

    GLint result = GL_FALSE;
    int infoLogLen;

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char* shaderErrMsg = new char[infoLogLen + 1];
        glGetShaderInfoLog(shaderID, infoLogLen, NULL, shaderErrMsg);
        std::cerr << shaderErrMsg << std::endl;
    }

    return shaderID;
}
GLuint linkShaderProgram(GLuint vert_shader, GLuint frag_shader)
{
    std::cout << "Linking shader program" << std::endl;

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vert_shader);
    glAttachShader(programID, frag_shader);
    glLinkProgram(programID);

    GLint result = GL_FALSE;
    int infoLogLen;

    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        char* programErrMsg = new char[infoLogLen + 1];
        glGetProgramInfoLog(programID, infoLogLen, NULL, programErrMsg);
        std::cerr << programErrMsg << std::endl;
    }

    glDetachShader(programID, vert_shader);
    glDetachShader(programID, frag_shader);

    return programID;
}

