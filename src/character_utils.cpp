// This was copied from https://learnopengl.com/In-Practice/Text-Rendering
// and then modifying it slightly the main modification being wrapping it
// in a class. But the core logic was written by Joey de Vries. This file
// is therefore licensed under the terms of the CC BY 4.0. You can read
// about it at https://creativecommons.org/licenses/by/4.0/ or find the
// full license at https://creativecommons.org/licenses/by/4.0/legalcode
// This is not provided under any warranty
//
// (c) Joey de Vries
// (You can find him on twitter at https://twitter.com/JoeyDeVriez


#include <GL/glew.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>
#include <iostream>


#include "character_utils.h"


Font::Font(const char* font)
    : LoadError(false), m_Characters()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Error: Failed to initialize FreeType2" << std::endl;
        LoadError = true;
        return;
    }

    std::cout << "Loading font " << font << std::endl;
    FT_Face face;
    if (FT_New_Face(ft, font, 0, &face)) {
        std::cerr << "Error: Failed to load font" << std::endl;
        LoadError = true;
        return;
    }

    SetupCharMap(face);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Font::SetupCharMap(FT_Face &face)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    FT_Set_Pixel_Sizes(face, 0, 48);

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Error: Failed to load " << std::hex << (uint8_t)c << " character" << std::endl;
            continue;
        }
        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        m_Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Font::RenderText(GLuint VAO, GLuint VBO, GLuint shaderProgram, const char* text, float x, float y, float scale)
{
    // activate corresponding render state	
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    for (size_t i = 0; text[i] != '\0'; i++)
    {
        char c = text[i];
        Character ch = m_Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


