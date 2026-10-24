#include "Player.h"

Player::Player(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint); // 去掉原生边框

    connect(ui.btnClose, &QPushButton::clicked, this, &Player::close); // 关联标题栏按钮到窗体槽函数
    connect(ui.btnMin, &QPushButton::clicked, this, &Player::showMinimized);
    connect(ui.btnMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
    });

    // 构造函数中连接点击事件
    connect(ui.btnOpen, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "选择图片", "", "图片文件 (*.jpg *.jpeg *.png *.bmp *.webp)"
        );
        if (filePath.isEmpty()) return;

        // 1. 调用独立解码模块解码图片
        ImageFrame frame = ImageDecoder::decode(filePath);
        if (!frame.isValid) {
            QMessageBox::warning(this, "错误", "FFmpeg 解码失败！");
            return;
        }

        // 2. 切走欢迎页，显示 OpenGL 渲染页
        ui.stackedWidget->setCurrentIndex(1); // 切到第2页（OpenGL 所在页）

        // 3. 将解码出的数据喂给 OpenGL 进行着色器渲染！
        ui.openGLWidget->renderFrame(frame.rgbData, frame.width, frame.height);
    });
}

Player::~Player()
{}

void Player::mousePressEvent(QMouseEvent* event) { // 实现拖动标题栏移动窗口
    if (event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void Player::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}