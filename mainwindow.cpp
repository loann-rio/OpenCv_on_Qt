#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myvideocapture.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mOpenCV_videoCapture = new MyVideoCapture(this);

    connect(mOpenCV_videoCapture, &MyVideoCapture::newPixmapCaptured, this, [&]()
    {
       ui->opencvFrame->setPixmap(mOpenCV_videoCapture->pixmap().scaled(696, 464));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
    mOpenCV_videoCapture->terminate();
}


void MainWindow::on_initOpenCvButton_clicked()
{
    mOpenCV_videoCapture->start(QThread::HighestPriority);
}


