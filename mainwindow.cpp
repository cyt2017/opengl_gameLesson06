#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString name[6];
    std::vector<QString> fileName;
    name[0] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex32X32.bmp";
    name[1] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex16X16.bmp";
    name[2] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex8X8.bmp";
    name[3] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex4X4.bmp";
    name[4] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex2X2.bmp";
    name[5] = "/home/cty/workspace/OPENGL/opengl_gameLesson06/build/tex1X1.bmp";
    for(int i=0;i<6;i++)
    {
        fileName.push_back(name[i]);
    }

    ui->widget->init_QGW(fileName);
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer->start(20);

    ui->widget->parent = this;
    this->installEventFilter(ui->widget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTimeOut()
{
    ui->widget->drawImage();
}
