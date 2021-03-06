#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void smooth(Mat& matIn, Mat& dest);
    void sobel(Mat& matIn, Mat& dest);
    void sharpen(Mat& matIn, Mat& dest);
    ~MainWindow();

private slots:
    void displayFrame();


    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    Mat frame;
    Mat frameIn;
    Mat previous;
    Mat current;
    Mat dragLines;
    VideoCapture cam;
    VideoCapture cam2;
    bool showBackground = true;
};

#endif // MAINWINDOW_H
