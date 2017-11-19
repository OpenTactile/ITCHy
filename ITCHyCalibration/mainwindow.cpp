#include "mainwindow.h"
#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QVector2D>
#include <QDebug>

#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    connect(buttonConnect, SIGNAL(clicked(bool)),
            this, SLOT(connectPressed()));
    connect(&timer, SIGNAL(timeout()),
            this, SLOT(pollMouse()));

    connect(&colortimer, SIGNAL(timeout()),
            this, SLOT(updateColor()));

    connect(buttonSave, SIGNAL(clicked(bool)),
            this, SLOT(updateEEPROM()));

    connect(sendCalibration, SIGNAL(clicked(bool)),
            this, SLOT(updateCalibration()));
    connect(sendPhysics, SIGNAL(clicked(bool)),
            this, SLOT(updateParameters()));

    connect(startCalibrationButton, SIGNAL(clicked(bool)),
            this, SLOT(startCalibration()));

    scene = new QGraphicsScene(-10.0, -10.0, 20, 20);
    canvas->setScene(scene);

    QPolygonF poly;
    poly << QPointF(0, 30) << QPointF(-30, 0) << QPointF(-15, 0)
            << QPointF(-15, -30) << QPointF(15, -30) << QPointF(15,0)
               << QPointF(30,0) << QPointF(0,30);

    mouseGraphic = scene->addPolygon(poly, QPen(), QBrush(Qt::gray));
    mouseGraphic->setScale(-1);

    mouseVelocity = scene->addLine(0,0,0,0, QPen(QBrush(Qt::red), 3.0, Qt::SolidLine, Qt::RoundCap));
    mouseAngular =  scene->addEllipse(-30,-30,60,60,Qt::NoPen,QBrush(QColor(255,0,0,64)));

    leftPosition = scene->addEllipse(0.0, 0.0, 10.0, 10.0, QPen(), QBrush(Qt::green));
    rightPosition = scene->addEllipse(0.0, 0.0, 10.0, 10.0, QPen(), QBrush(Qt::cyan));

    calibRect = scene->addRect(0,0,spinCalibrationWidth->value()*1000.0* 0.01, -spinCalibrationHeight->value()*1000.0* 0.01);

    canvas->centerOn(calibRect->boundingRect().center());

    mouse.addCallback(ITCHy::CallbackType::Disconnected, [&](){disconnect();});

    canvas->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    scene->setSceneRect(calibRect->rect());
    canvas->fitInView(calibRect, Qt::KeepAspectRatio);

    pollMouse();
}

void MainWindow::connectPressed()
{
    statusLabel->setText("Connecting...");
    qApp->processEvents();

    mouse.connect();

    statusLabel->setText("Connected");
    timer.setInterval(10);
    timer.start();

    colortimer.setInterval(50);
    colortimer.start();
    buttonConnect->setEnabled(false);
}

void MainWindow::disconnected()
{
    timer.stop();
    statusLabel->setText("Disconnected");
    buttonConnect->setEnabled(true);
}

void MainWindow::pollMouse()
{
    const ITCHy::State& state = mouse.currentState();
    if(state.time != 0)
        return;

    qDebug() << state.position[0] << "," << state.position[1] << " / " << state.angle;
    qDebug() << state.leftIncrement[0] << "\t" << state.leftIncrement[1] << "\t/\t" << state.rightIncrement[0] << "\t" << state.rightIncrement[1];
    qDebug() << state.leftSensor[0] << "\t" << state.leftSensor[1] << "\t/\t" << state.rightSensor[0] << "\t" << state.rightSensor[1];
    qDebug() << "v:\t" << state.velocity[0] << "\t" << state.velocity[1];
    qDebug() << "a:\t" << state.angle;
    qDebug() << "---\n\n\n";

    leftPosition->setPos(QPointF(state.leftSensor[0], -state.leftSensor[1])*1000.0 - QPointF(5, 5));
    rightPosition->setPos(QPointF(state.rightSensor[0], -state.rightSensor[1])*1000.0 - QPointF(5, 5));

    //leftRaw->setPos(QPointF(state.leftRaw[0], -state.leftRaw[1])*1000.0 - QPointF(2.5, 2.5));
    //rightRaw->setPos(QPointF(state.rightRaw[0], -state.rightRaw[1])*1000.0 - QPointF(2.5, 2.5));

    mouseGraphic->setPos(QPointF(state.position[0], -state.position[1])*1000.0);
    float angle = -state.angle / M_PI * 180.0;
    mouseGraphic->setRotation(angle);

    mouseVelocity->setPos(mouseGraphic->pos());
    mouseVelocity->setLine(0,0, state.velocity[0] * 100.0, -state.velocity[1]*100.0);

    mouseAngular->setPos(mouseGraphic->pos());
    mouseAngular->setRotation(angle);

    mouseAngular->setStartAngle(16*90);
    if(mouseAngular->rotation() >= 0.0)
        mouseAngular->setSpanAngle(-state.angularVelocity*1000000);
    else
        mouseAngular->setSpanAngle(state.angularVelocity*1000000);

    labelDt->setText(QString("dt = ") + QString::number(state.timeStep));

    scene->setSceneRect(calibRect->rect());
    canvas->fitInView(calibRect->rect().adjusted(-2, -2, 2, 2), Qt::KeepAspectRatio);

    // Change color on thumb button press:
    if(state.button == 1)
    {
        mouse.setColor({{200, 255, 200}});
    }
}

void MainWindow::updateColor()
{
    if(mouse.currentState().button == 1)
        return;

    clt += 0.01;
    if(clt > 1.0)
        clt -= 1.0;
    QColor cl = QColor::fromHsvF(clt, 1.0, 1.0, 1.0);
    mouse.setColor({{uint8_t(cl.red()),
                     uint8_t(cl.green()),
                     uint8_t(cl.blue())}});
}

void MainWindow::updateParameters()
{
    mouse.setSimulationParameters(
                spinMass->value(),
                spinStiffness->value(),
                spinDamping->value(),
                spinUpdates->value()
                );
}

void MainWindow::updateCalibration()
{
    mouse.setCalibrationParameters({{
                                        float(spinCalibrationWidth->value() * 0.01),
                                        float(spinCalibrationHeight->value() * 0.01)
                                    }});
}

void MainWindow::startCalibration()
{
    mouse.startCalibration();
}

void MainWindow::updateEEPROM()
{
    mouse.saveState();
}
