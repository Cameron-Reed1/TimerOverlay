#include <GL/glew.h>
#include <glm/glm.hpp>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    FT_Pos Advance;          // Offset to advance to next glyph
};

class Font
{
public:
    Font() = delete;
    Font(const char* font);
    void RenderText(GLuint VAO, GLuint VBO, GLuint shaderProgram, const char* text, float x, float y, float scale);

private:
    void SetupCharMap(FT_Face& face);

public:
    bool LoadError;

private:
    std::unordered_map<GLchar, Character> m_Characters;
};

