#include "Player.h"

Player::Player(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    // 1. 去掉原生边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);

    // 2. 关联标题栏按钮到窗体槽函数
    connect(ui.btnClose, &QPushButton::clicked, this, &Player::close);
    connect(ui.btnMin, &QPushButton::clicked, this, &Player::showMinimized);
    connect(ui.btnMax, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
    });
}

Player::~Player()
{}

// 3. 实现拖动标题栏移动窗口
void Player::mousePressEvent(QMouseEvent* event) {
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