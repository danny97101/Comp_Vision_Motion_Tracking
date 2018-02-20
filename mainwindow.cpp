#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cam=VideoCapture(0);
    cam2=VideoCapture(1);
    ui->label->setScaledContents(true);

    QTimer *qTimer=new QTimer(this);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(displayFrame()));
    qTimer->start(10);
    cam>>current;
    dragLines = Mat(current.rows, current.cols, current.type());
    previous = Mat();
    current = Mat();
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

inline int dist(Vec3b& a, Vec3b& b){
    return abs(a[0]-b[0])+abs(a[1]-b[1])+abs(a[2]-b[2]);
}

void MainWindow::displayFrame(){
    // fade drag lines
    int fadeSpeed = ui->horizontalSlider->value();
    for (int row = 0; row < dragLines.rows; row++) {
        Vec3b* pixel = dragLines.ptr<cv::Vec3b>(row); // point to first pixel in row
        for (int col = 0; col < dragLines.cols; col++) {
            if ((*pixel)[0] > 0) {
                if ((*pixel)[0] >= fadeSpeed) {
                    (*pixel)[0] = (*pixel)[0] - fadeSpeed;
                    //(*pixel)[1] = (*pixel)[1] + fadeSpeed;
                }
                else
                    (*pixel)[0] = 0;
            }
            pixel++;
        }
    }

    previous = current.clone();
    cam>>frameIn;
    cvtColor(frameIn, current, CV_BGR2RGB);
    if (showBackground)
        frame = current.clone();
    else {
        frame = Mat(current.rows,current.cols,current.type());
        frame.setTo(Scalar(0,0,0));
    }

    if (previous.empty()) return;

    for (int row = 0; row < current.rows; row++) {
        Vec3b* pixel = current.ptr<cv::Vec3b>(row); // point to first pixel in row
        Vec3b* prev = previous.ptr<cv::Vec3b>(row);
        Vec3b* dragLinePix = dragLines.ptr<cv::Vec3b>(row);
        Vec3b* framePix = frame.ptr<cv::Vec3b>(row);
        for (int col = 0; col < current.cols; col++) {
            if (dist(*pixel, *prev) > ui->horizontalSlider_2->value()) {
                dragLinePix[0] = 255;
            }

            if ((*dragLinePix)[0] > 0) {
                (*framePix)[0] = (*dragLinePix)[0];
                (*framePix)[1] = 0;
                (*framePix)[2] = 0;
            }

            framePix++;
            dragLinePix++;
            pixel++;
            prev++;
        }
    }


    QImage im = QImage((const unsigned char*)(frame.data),frame.cols,frame.rows,frame.step,QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(im));
}

void MainWindow::on_pushButton_clicked()
{
    showBackground = !showBackground;
}
