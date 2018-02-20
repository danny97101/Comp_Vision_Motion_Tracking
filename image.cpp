#include "image.h"
#include <iostream>

Image::Image()
{
    blankFrame= Mat();
}


int* findClosestBall(int** previous, int numBalls, int* ball) {
    int closestBall = -1;
    int minSquareDist = -1;
    for (int i = 0; i < numBalls; i++) {
        int squareDistance = (ball[0]-previous[i][0])*(ball[0]-previous[i][0]) + (ball[1]-previous[i][1])*(ball[1]-previous[i][1]);
        if (minSquareDist == -1 || squareDistance < minSquareDist) {
            if (squareDistance < 30) {
                closestBall = i;
                minSquareDist = squareDistance;
            }
        }
    }
    if (closestBall != -1) {
        return previous[closestBall];
    } else {
        return nullptr;
    }
}

void Image::setCameraNum(int i){
    cam=VideoCapture(i);
    cam.set(CV_CAP_PROP_AUTOFOCUS,0);
    cam.set(CV_CAP_PROP_AUTO_EXPOSURE,0);
}

void Image::takePicture(){
    for(int i=0;i<1;i++){
        cam>>src;
    }
}

void Image::readFromFile(String dataDirectory) {
    src = imread(dataDirectory + std::to_string(readCount) + ".bmp", IMREAD_COLOR);
    if (src.empty()) {
        readCount--;
        src = imread(dataDirectory + std::to_string(readCount) + ".bmp", IMREAD_COLOR);
    }
    readCount++;
}

void Image::takePicture(double focus,double exposure){
    for(int i=0;i<1;i++){
        cam>>src;
    }
    cam.set(CV_CAP_PROP_FOCUS,focus);
}


inline int dist(Vec3b& a, Vec3b& b){
    return abs(a[0]-b[0])+abs(a[1]-b[1])+abs(a[2]-b[2]);
}

void Image::decisionFilter(Mat &a, Mat &b)
{
    for (int i = 0; i < a.rows; i++){
        //printf("I: %d | ",i);
        Vec3b* pixela = a.ptr<cv::Vec3b>(i); // point to first pixel in row
        Vec3b* pixelb = b.ptr<cv::Vec3b>(i); // point to first pixel in row
        for (int j = 0; j < a.cols; j++){
            //printf("%d,",j);
            if(dist(*pixela,*pixelb)<thresh1){
                (*pixela)[1]=0;
                (*pixela)[2]=0;
            }
            // increment pointers
            pixela++;
            pixelb++;
        }
        //std::cout<<std::endl;
    }
//    for(int y=1;y<a.rows-1;y++){
//        for(int x=0;x<a.cols;x++){
//            for(int c=0;c<3;c++){
////                int c1=(int)in.at<Vec3b>(y,x+1)[c];
////                int c2=(int)in.at<Vec3b>(y,x-1)[c];
////                char c3=(char)(127+c1-c2);
//                //printf("Colors %d %d %u, ",c1,c2,c3);

//                a.at<Vec3b>(y,x)[c]=50;
//            }
//        }
//    }
}


QImage Image::convertToQImage(){
    cv::cvtColor(im,display,CV_BGR2RGB);
    //display= im;
    return QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
}


void Image::displayImage(QLabel &location){
    QImage img=convertToQImage();
    location.setPixmap(QPixmap::fromImage(img));
}



void Image::processImage(Mat& other){
    for(int y=0;y<src.rows;y++){
        for(int x=0;x<src.cols;x++){
            Vec3b sc = src.at<Vec3b>(y,x);
            Vec3b oc = other.at<Vec3b>(y,x);
            int whiteness=abs(sc[0]-sc[1])+abs(sc[1]-sc[2])+abs(sc[2]-sc[0]);
            int brightness=sc[0]+sc[1]+sc[2];
            //im.at<Vec3b>(y,x)=(y%2==0)?sc:oc;
            im.at<Vec3b>(y,x)=(whiteness>wThresh||brightness<bThresh)?sc:oc;//G
//            im.at<Vec3b>(y,x)[0]=0;//B
//            im.at<Vec3b>(y,x)[1]=(whiteness>30||brightness<300)?127:0;//G
//            im.at<Vec3b>(y,x)[2]=0;//R
        }
    }
}

void Image::processImage(){
    //cv::GaussianBlur(src,im,Size(5,5),5);
    src.copyTo(im);
    if(blankFrame.empty()){
        printf("Initing blank frame\n");
        im.copyTo(blankFrame);
        //blankFrame*=.5;
    }
    //im=(blankFrame-im)+(im-blankFrame);
    decisionFilter(im,blankFrame);
    //Un-comment in order to recollect data.
    //imwrite("/home/paul/bocce/dataset_game/"+std::to_string(ctr++)+".bmp",im);
}

void Image::invert(Mat& mat){
    mat=Scalar(255,255,255)-mat;
//    for(int y=0;y<mat.rows;y++){
//        for(int x=0;x<mat.cols;x++){
//            for(int c=0;c<3;c++){
//                mat.at<Vec3b>(y,x)[c]=255-mat.at<Vec3b>(y,x)[c];
//            }
//        }
//    }
}

void Image::sobel(Mat& in,Mat& out, int dx, int dy){
    dx=dx<0?-dx:dx;
    dy=dy<0?-dy:dy;
    out.setTo(Scalar(0,0,0));
//    mat=Scalar(255,255,255)-mat;
    for(int y=dy;y<in.rows-dy;y++){
        for(int x=dx;x<in.cols-dx;x++){
            for(int c=0;c<3;c++){
                for(int sx=-dx;sx<=dx;sx++){
                    for(int sy=-dy;sy<=dy;sy++){
                        out.at<Vec3b>(y,x)[c]=(in.at<Vec3b>(y,x)[c]-in.at<Vec3b>(y+sy,x+sx)[c]);
                    }
                }
            }
        }
    }
}

void Image::sobelv(Mat& in,Mat& out){
//    out.setTo(Scalar(127,127,127));
//    mat=Scalar(255,255,255)-mat;
    for(int y=1;y<in.rows-1;y++){
        for(int x=0;x<in.cols;x++){
            for(int c=0;c<3;c++){
//                int c1=(int)in.at<Vec3b>(y,x+1)[c];
//                int c2=(int)in.at<Vec3b>(y,x-1)[c];
//                char c3=(char)(127+c1-c2);
                //printf("Colors %d %d %u, ",c1,c2,c3);

                out.at<Vec3b>(y,x)[c]=127+(in.at<Vec3b>(y+1,x)[c]-in.at<Vec3b>(y-1,x)[c]);//(in.at<Vec3b>(y-1,x)[c]-in.at<Vec3b>(y+1,x)[c]);
            }
        }
    }
}

void Image::sobelh(Mat& in,Mat& out){
//    out.setTo(Scalar(127,127,127));
//    mat=Scalar(255,255,255)-mat;
    for(int y=0;y<in.rows;y++){
        for(int x=1;x<in.cols-1;x++){
            for(int c=0;c<3;c++){
//                int c1=(int)in.at<Vec3b>(y,x+1)[c];
//                int c2=(int)in.at<Vec3b>(y,x-1)[c];
//                char c3=(char)(127+c1-c2);
                //printf("Colors %d %d %u, ",c1,c2,c3);

                out.at<Vec3b>(y,x)[c]=127+(in.at<Vec3b>(y,x+1)[c]-in.at<Vec3b>(y,x-1)[c]);//(in.at<Vec3b>(y-1,x)[c]-in.at<Vec3b>(y+1,x)[c]);
            }
        }
    }
}

void Image::absdv(Mat& in,Mat& out){
    out=abs(in-Scalar(127,127,127));
}
int abs(int n){
    return n<0?-n:n;
}

void Image::threshold(Mat& in, Mat& out,int thresh){
    for(int y=0;y<in.rows;y++){
        for(int x=0;x<in.cols;x++){
            for(int c=0;c<3;c++){
                out.at<Vec3b>(y,x)[c]=(out.at<Vec3b>(y,x)[c]>thresh)?0:255;
            }
        }
    }
}

Mat& Image::getSrc(){
    return src;
}

Mat& Image::getIm(){
    return im;
}
