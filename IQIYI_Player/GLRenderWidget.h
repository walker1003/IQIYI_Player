#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <vector>

class GLRenderWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit GLRenderWidget(QWidget* parent = nullptr);
    ~GLRenderWidget() override;

    // 接收解码数据并触发重绘
    void renderFrame(const std::vector<uint8_t>& rgbData, int width, int height);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram m_program;
    GLuint m_textureId = 0;
    GLuint m_vbo = 0;

    std::vector<uint8_t> m_currentFrameData;
    int m_videoWidth = 0;
    int m_videoHeight = 0;
    bool m_hasNewFrame = false;
};