#pragma once
#include <QPoint>
#include <QMouseEvent>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "../ImageDecoder/ImageDecoder.h"
#include "ui_Player.h"

class Player : public QMainWindow
{
    Q_OBJECT

public:
    Player(QWidget *parent = nullptr);
    ~Player();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private slots:
    void OnMaxBtnClickd();
    void OnOpenBtnClickd();
    void OnDrawerBtnClickd();

private:
    Ui::PlayerClass ui;
    QPoint m_dragPos;
};

