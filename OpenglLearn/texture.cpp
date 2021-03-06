#include "texture.h"
#include <iostream>

Texture::Texture(GLenum TextureTarget, const std::string &FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}

bool Texture::Load()
{
/*    try {
        m_image.read(m_fileName);
        m_image.write(&m_blob, "RGBA");
    }
    catch (Magick::Error& Error) {
        std::cout << "Error loading texture '" << m_fileName << "': " << Error.what() << std::endl;
        return false;
    }
*/
    QString filename = QString::fromStdString(m_fileName);
    m_image.load(filename);

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.width(), m_image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(m_textureTarget, 0);

    return true;
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
