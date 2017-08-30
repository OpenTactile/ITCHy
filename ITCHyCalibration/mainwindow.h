#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <itchy/itchy.h>
#include <QTimer>
#include <QGraphicsScene>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void connectPressed();
    void pollMouse();

    void disconnected();
    void updateParameters();
    void updateCalibration();
    void startCalibration();
    void updateEEPROM();

    void updateColor();

private:
    ITCHy mouse;
    QTimer timer;
    QTimer colortimer;
    float clt = 0.0;

    QGraphicsScene* scene;

    QGraphicsEllipseItem* leftPosition;
    QGraphicsEllipseItem* rightPosition;

    QGraphicsPolygonItem* mouseGraphic;
    QGraphicsLineItem* mouseVelocity;
    QGraphicsEllipseItem* mouseAngular;
    QGraphicsRectItem* calibRect;
};

#endif // MAINWINDOW_H
