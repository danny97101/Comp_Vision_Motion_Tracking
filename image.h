#ifndef IMAGE_H
#define IMAGE_H

#include <opencv/cv.hpp>
#include <opencv/highgui.h>


#include <QLabel>
#include <QMainWindow>
#include <QtCore>

using namespace cv;

class Image
{
public:
    Image();
    void setCameraNum(int i);
    void takePicture();
    QImage convertToQImage();
    void displayImage(QLabel &location);
    void processImage(Mat& other);
    void processImage();
    Mat& getSrc();
    static void sobel(Mat& in,Mat& out, int dx, int dy);
    static void sobelh(Mat& in,Mat& out);
    static void sobelv(Mat& in,Mat& out);
    static void absdv(Mat& in,Mat& out);
    static void threshold(Mat& in, Mat& out,int thresh);
    int wThresh=30;
    int bThresh=300;
    void takePicture(double focus,double exposure);
    void decisionFilter(Mat& a, Mat& b);
    void readFromFile(String dataDirectory);
    Mat& getIm();
    void invert(Mat& im);
    int* findClosestBall(int** previous, int numBalls, int* ball);

    float thresh1=0, thresh2=0, thresh3=0, thresh4=0;
private:
    Mat src;
    Mat display;
    Mat im;
    Mat blankFrame;
    VideoCapture cam;
    int ctr=0;
    int readCount=0;
};

#endif // IMAGE_H
