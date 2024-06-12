#include <GL/glew.h>


GLuint loadShader(const char* const file_path, GLenum shader_type);
GLuint loadShaderDirect(const char* const shader_src, GLenum shader_type);
GLuint linkShaderProgram(GLuint vert_shader, GLuint frag_shader);
