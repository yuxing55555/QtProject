#ifndef GL_BUFFEROBJECT_H
#define GL_BUFFEROBJECT_H

#include "gutils.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  class BufferObject
  {
  public:
    static void Unbind (GLenum target);

    enum TYPES
    {
      VERTEXBUFFEROBJECT = GL_ARRAY_BUFFER,
      INDEXBUFFEROBJECT = GL_ELEMENT_ARRAY_BUFFER,
    };

    BufferObject (GLenum target);
    ~BufferObject ();

    void Bind ();
    void Unbind ();

    //VBO: Bind the VBO to a VAO
    //IBO: Bind the IBO to the VAO
    void SetBufferData (GLsizeiptr size, const GLvoid *data, GLenum usage);

    GLuint GetID ();

  private:
    GLuint m_id;
    GLenum m_target;
  };
}

#endif