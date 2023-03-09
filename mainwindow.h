#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui
{
class MainWindow;
}

class MyVideoCapture;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_initOpenCvButton_clicked();

private:
    Ui::MainWindow *ui;
    MyVideoCapture *mOpenCV_videoCapture;
};
#endif // MAINWINDOW_H
