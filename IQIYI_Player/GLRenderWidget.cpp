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
    // 1. 获取物理像素宽高（兼容 125%、150% 等 Windows 高分屏缩放）
    int winWidth = this->width() * this->devicePixelRatio();
    int winHeight = this->height() * this->devicePixelRatio();

    // 2. 先将整个视口清空为纯黑背景
    glViewport(0, 0, winWidth, winHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    // 如果还没有加载图片或图片尺寸非法，则不渲染画面
    if (m_currentFrameData.empty() || m_videoWidth <= 0 || m_videoHeight <= 0) {
        return;
    }

    // 3. 计算保持原始宽高比的居中显示区域
    float imgRatio = (float)m_videoWidth / (float)m_videoHeight;
    float winRatio = (float)winWidth / (float)winHeight;

    int renderX = 0, renderY = 0;
    int renderW = winWidth, renderH = winHeight;

    if (winRatio > imgRatio) {
        // 窗口比图片更宽：高度铺满，左右自适应留黑边
        renderH = winHeight;
        renderW = (int)(winHeight * imgRatio);
        renderX = (winWidth - renderW) / 2;
        renderY = 0;
    }
    else {
        // 窗口比图片更高：宽度铺满，上下自适应留黑边
        renderW = winWidth;
        renderH = (int)(winWidth / imgRatio);
        renderX = 0;
        renderY = (winHeight - renderH) / 2;
    }

    // 4. 将绘制区域限制在计算出的居中视口内
    glViewport(renderX, renderY, renderW, renderH);

    m_program.bind();

    // 如果有新数据到达，上传纹理到显卡
    if (m_hasNewFrame) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 1字节对齐防止花屏
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_videoWidth, m_videoHeight,
            0, GL_RGB, GL_UNSIGNED_BYTE, m_currentFrameData.data());
        m_hasNewFrame = false;
    }

    // 绑定并激活纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_program.setUniformValue("uTexture", 0);

    // 绘制顶点（四边形）
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