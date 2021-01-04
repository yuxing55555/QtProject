#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include"raytraceinfo.h"
#include"showimage.h"
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <math.h>
#include <QImage>
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
using namespace std;
GLuint  compute_prog;
GLuint  compute_shader;
GLuint  render_prog;
GLuint  render_vbo;
GLuint m_textureObj;
int texture_width = 256;
int texture_height = 256;

bool ReadFile(const char* pFileName, string& outFile)
{
    ifstream f(pFileName);
    bool ret = false;

    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }
        f.close();
        ret = true;
    }
    else {
        printf("read file error!");
    }

    return ret;
}
// 使用shader文本编译shader对象，并绑定shader都想到着色器程序中
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    //创建shader着色器对象
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    //在编译shader对象之前我们必须先定义它的代码源
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0]= strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

    //编译shader对象
    glCompileShader(ShaderObj);

    //获得编译状态，并且可以打印编译器碰到的所有编译错误
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    //将编译好的shader对象绑定在program object程序对象上
    glAttachShader(ShaderProgram, ShaderObj);
}

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(compute_prog);
    glBindImageTexture(0, m_textureObj, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(texture_width, texture_height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    glUseProgram(render_prog);
    glEnableVertexAttribArray(0);//开启顶点属性
    glEnableVertexAttribArray(1);//开启顶点属性

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureObj);//绑定纹理

    // 告诉管线怎样解析bufer中的数据
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);//禁用顶点数据
    glDisableVertexAttribArray(1);//禁用顶点数据

    glutSwapBuffers();
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
}

bool LoadTexture()
{

    glGenTextures(1, &m_textureObj);//生成纹理队形
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureObj);//绑定对象

    //放大和缩小指定过滤方式
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //纹理环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texture_width, texture_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, m_textureObj, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return true;
}

static void InitData(){
    string s;//存储着色器文本的字符串缓冲
    const char* computer_cs = "./computer.cs";
    if (!ReadFile(computer_cs, s)) {
        return;
    }

    compute_prog = glCreateProgram();
    AddShader(compute_prog, s.c_str(), GL_COMPUTE_SHADER);
    glLinkProgram(compute_prog);

    render_prog = glCreateProgram();

    static const char render_vs[] =
        "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec2 aPosition;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2    texCoord;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vec4 pos = vec4(aPosition, 1.0F, 1.0F);\n"
        "    gl_Position = pos;\n"
        "    texCoord = aTexCoord;\n"
        "}\n";

    static const char render_fs[] =
        "#version 430 core\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "in vec2        texCoord;\n"
        "uniform sampler2D output_image;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = texture(output_image, texCoord);\n"
        "}\n";

    AddShader(render_prog, render_vs, GL_VERTEX_SHADER);
    AddShader(render_prog, render_fs, GL_FRAGMENT_SHADER);
    glLinkProgram(render_prog);

    glGenBuffers(1, &render_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
    static const float verts[] =
    {
        -1.f, -1.f,
        0.f, 1.f,
        -1.f, 1.f,
        0.f, 0.f,
        1.f, -1.f,
        1.f, 1.f,
        -1.f, 1.f,
        0.f, 0.f,
        1.f, -1.f,
        1.f, 1.f,
        1.f, 1.f,
        1.f, 0.f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    LoadTexture();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("opengl learn");

    InitializeGlutCallbacks();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    InitData();
    glutMainLoop();
    return 0;
/*
    QApplication a(argc, argv);

    qDebug() << "begin ray:";
    RayTraceInfo ray;
    ray.updateImage(ray.m_direction);

    ShowImage label;
    label.setWindowTitle("label");
    label.rayinfo = &ray;
    label.updateLabel(ray.m_winsize[0],ray.m_winsize[1],ray.m_imageptr);
    label.show();
    label.showready = true;

    return a.exec();
*/
}
