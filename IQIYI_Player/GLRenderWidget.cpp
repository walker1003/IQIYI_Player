#include "GLRenderWidget.h"

// 顶点着色器（顶点位置 + 纹理坐标）
static const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

// 片元着色器（从 RGB 纹理采样）
static const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D uTexture;
    void main() {
        FragColor = texture(uTexture, TexCoord);
    }
)";

GLRenderWidget::GLRenderWidget(QWidget* parent) : QOpenGLWidget(parent) {}

GLRenderWidget::~GLRenderWidget() {
    makeCurrent();
    if (m_textureId) glDeleteTextures(1, &m_textureId);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    doneCurrent();
}

void GLRenderWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 纯黑底色

    // 编译着色器
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program.link();

    // 全屏矩形顶点（翻转了 Y 轴的纹理坐标，防止画面上下颠倒）
    static const float vertices[] = {
        // 顶点坐标    // 纹理坐标
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 0.0f
    };

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 创建纹理
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLRenderWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GLRenderWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_currentFrameData.empty() || m_videoWidth <= 0 || m_videoHeight <= 0) {
        return;
    }

    m_program.bind();

    // 如果有新帧到达，上传纹理到显卡
    if (m_hasNewFrame) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        // 【极其重要】设置 1 字节对齐，防止奇数宽度的图片花屏/倾斜
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_videoWidth, m_videoHeight,
            0, GL_RGB, GL_UNSIGNED_BYTE, m_currentFrameData.data());
        m_hasNewFrame = false;
    }

    // 绑定并激活纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_program.setUniformValue("uTexture", 0);

    // 绘制全屏四边形
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    m_program.release();
}

void GLRenderWidget::renderFrame(const std::vector<uint8_t>& rgbData, int width, int height) {
    m_currentFrameData = rgbData;
    m_videoWidth = width;
    m_videoHeight = height;
    m_hasNewFrame = true;
    update(); // 触发 paintGL 重新绘制
}