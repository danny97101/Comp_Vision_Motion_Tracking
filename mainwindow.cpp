#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cam=VideoCapture(0);
    cam2=VideoCapture(1);
    ui->imageOut->setScaledContents(true);
    ui->imageOut_2->setScaledContents(true);
    ui->imageOut_3->setScaledContents(true);
    ui->imageOut_4->setScaledContents(true);

    QTimer *qTimer=new QTimer(this);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(displayFrame()));
    qTimer->start(10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::smooth(Mat& matIn, Mat &dest) {
    for (int row = 1; row < matIn.rows - 1; row++) {
        for (int col = 1; col < matIn.cols - 1; col++) {
            Vec3b topLeft = matIn.at<Vec3b>(row-1, col-1);
            Vec3b topMid = matIn.at<Vec3b>(row-1, col);
            Vec3b topRight = matIn.at<Vec3b>(row-1, col+1);
            Vec3b midLeft = matIn.at<Vec3b>(row, col-1);
            Vec3b mid = matIn.at<Vec3b>(row,col);
            Vec3b midRight = matIn.at<Vec3b>(row, col+1);
            Vec3b botLeft = matIn.at<Vec3b>(row+1,col-1);
            Vec3b botMid = matIn.at<Vec3b>(row+1,col);
            Vec3b botRight = matIn.at<Vec3b>(row+1, col+1);
            for (int color = 0; color < 3; color++)
                dest.at<Vec3b>(row,col)[color] = (topLeft[color] + 2*topMid[color] + topRight[color] +
                                                    2*midLeft[color] + 4*mid[color] + 2*midRight[color] +
                                                    botLeft[color] + 2*botMid[color] + botRight[color]) / 16;

        }
    }
}

void MainWindow::sobel(Mat &matIn, Mat &dest) {
    Mat smooth = Mat(matIn.rows,matIn.cols,matIn.type());
    MainWindow::smooth(matIn, smooth);
    QImage smoothed=QImage((const unsigned char*)(smooth.data),smooth.cols,smooth.rows,
                      smooth.step,QImage::Format_RGB888);
    //ui->imageOut->setPixmap(QPixmap(QPixmap::fromImage(smoothed)));

    Mat grey;
    cvtColor(smooth, grey, CV_RGB2GRAY);
    QImage gray=QImage((const unsigned char*)(grey.data),grey.cols,grey.rows,
                      grey.step,QImage::Format_Grayscale8);
    //ui->imageOut_4->setPixmap(QPixmap(QPixmap::fromImage(gray)));

    for (int row = 1; row < matIn.rows - 1; row++) {
        for (int col = 1; col < matIn.cols - 1; col++) {
            char topLeft = grey.at<char>(row-1, col-1);
            char topMid = grey.at<char>(row-1, col);

            char midLeft = grey.at<char>(row, col-1);

            char midRight = grey.at<char>(row, col+1);
            char botLeft = grey.at<char>(row+1,col-1);
            char botMid = grey.at<char>(row+1,col);

            int diff = abs(-2*topLeft - topMid - midLeft + midRight + botMid + 2*botLeft) / 2;
            if (diff > 50) {
                  dest.at<Vec3b>(row,col)[0] = 255;
                  dest.at<Vec3b>(row,col)[1] = 255;
                  dest.at<Vec3b>(row,col)[2] = 255;
            } else {
                dest.at<Vec3b>(row,col)[0] = 0;
                dest.at<Vec3b>(row,col)[1] = 0;
                dest.at<Vec3b>(row,col)[2] = 0;

            }
        }
    }
}

void MainWindow::sharpen(Mat &matIn, Mat &dest) {
    for (int row = 1; row < matIn.rows - 1; row++) {
        for (int col = 1; col < matIn.cols - 1; col++) {
            /*Vec3b topLeft = matIn.at<Vec3b>(row-1, col-1);
            Vec3b topMid = matIn.at<Vec3b>(row-1, col);
            Vec3b topRight = matIn.at<Vec3b>(row-1, col+1);
            Vec3b midLeft = matIn.at<Vec3b>(row, col-1);
            Vec3b mid = matIn.at<Vec3b>(row,col);
            Vec3b midRight = matIn.at<Vec3b>(row, col+1);
            Vec3b botLeft = matIn.at<Vec3b>(row+1,col-1);
            Vec3b botMid = matIn.at<Vec3b>(row+1,col);
            Vec3b botRight = matIn.at<Vec3b>(row+1, col+1);
            for (int color = 0; color < 3; color++)
                dest.at<Vec3b>(row,col)[color] = (-1*topLeft[color] - topMid[color] - topRight[color] -
                                                    midLeft[color] + 8*mid[color] - midRight[color] -
                                                    botLeft[color] - botMid[color] - botRight[color]) / 16;
*/          Vec3b mid = matIn.at<Vec3b>(row,col);
            for (int color = 0; color < 3; color++) {
                if (mid[color] > 127)
                    dest.at<Vec3b>(row,col)[color] = mid[color] + (255-mid[color])/10;
                else
                    dest.at<Vec3b>(row,col)[color] = .9 * mid[color];
            }
        }
    }
}

void MainWindow::displayFrame(){
    //for (int i=0;i<5;i++)
        cam>>frameIn;
    cvtColor(frameIn,frame,CV_BGR2RGB);
    /*    Mat backgroundIn, background;
        cam2>>backgroundIn;
        cvtColor(backgroundIn, background, CV_BGR2RGB);


    for (int row = 0; row < frame.rows; row++) {
        for (int col = 0; col < frame.cols; col++) {
            Vec3b pixel = frame.at<Vec3b>(row, col);
            if (pixel[0] > 160 && pixel[1] > 160 && pixel[2] > 160) {
                frame.at<Vec3b>(row,col)[0] = background.at<Vec3b>(row,col)[0];
                frame.at<Vec3b>(row,col)[1] = background.at<Vec3b>(row,col)[1];
                frame.at<Vec3b>(row,col)[2] = background.at<Vec3b>(row,col)[2];
            }
        }
    }
*/

    QImage orig = QImage((const unsigned char*)(frame.data),frame.cols,frame.rows,
                         frame.step,QImage::Format_RGB888);

    ui->imageOut->setPixmap(QPixmap(QPixmap::fromImage(orig)));

    Mat smoothed = Mat(frame.rows,frame.cols,frame.type());
    MainWindow::smooth(frame, smoothed);
    QImage smoothPic = QImage((const unsigned char*)(smoothed.data),smoothed.cols,smoothed.rows,
                         smoothed.step,QImage::Format_RGB888);

    ui->imageOut_2->setPixmap(QPixmap(QPixmap::fromImage(smoothPic)));

    Mat sharpened = Mat(frame.rows,frame.cols,frame.type());
    MainWindow::sharpen(smoothed, sharpened);
    QImage sharpenedPic = QImage((const unsigned char*)(sharpened.data),sharpened.cols,sharpened.rows,
                         sharpened.step,QImage::Format_RGB888);
    ui->imageOut_3->setPixmap(QPixmap(QPixmap::fromImage(sharpenedPic)));

}
