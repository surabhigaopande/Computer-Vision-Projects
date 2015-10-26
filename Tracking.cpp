/// Tracking Vehicles from the Bird's eye view of Video///
/////Surabhi Gaopande////

///OpenCV Headers///
#include "cv.h"
#include "highgui.h"
#include "cvaux.h"
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;
int n =0;
double p=0;
Mat src;
Mat src1;
Mat src_gray;
int thresh = 39;
int max_thresh = 255;
RNG rng(12345);
bool first=true;
/// Function header
void thresh_callback(int, void* );

/**
 * @function main
 */
int main( int argc, char** argv )
{

    CvCapture*   inputMovie;
    inputMovie   = cvCreateFileCapture("demo.mpg");
    IplImage*   colourImage;
    colourImage   = cvQueryFrame(inputMovie);
    IplImage*   movingAverage   = cvCreateImage(    cvGetSize( colourImage),   IPL_DEPTH_32F, 3);
    IplImage*   difference;
    IplImage*   temp;
    for(;;)
    {
        colourImage   =  cvQueryFrame(inputMovie);
        IplImage*   input  =  cvCloneImage(colourImage);
        if(  !colourImage   )
        {
            break;
        }
        if(first)
        {
            difference   = cvCloneImage(colourImage);
            temp  = cvCloneImage(colourImage);
            cvConvertScale(colourImage,      movingAverage,    1.0,  0.0);
            first  =  false;
        }
        else
        {
            cvRunningAvg(colourImage,      movingAverage,    0.020,  NULL);
        }
        cvConvertScale(movingAverage,       temp,  1.0,  0.0);
        cvAbsDiff(colourImage,temp,difference);
        cvSaveImage("round.jpg",difference,0);
        cvSaveImage("color.jpg",colourImage);
        /// Load source image and convert it to gray
        src = imread( "round.jpg", 1 );
        src1 = imread( "color.jpg", 1 );
        /// Convert image to gray and blur it
        cvtColor( src, src_gray, CV_BGR2GRAY );
        blur( src_gray, src_gray, Size(5,5) );
        cv::GaussianBlur(src_gray,src_gray,cv::Size(5,5),1.5);
        /// Create Window
        char* source_window = "Source";
        namedWindow( source_window, CV_WINDOW_AUTOSIZE );
        imshow( source_window, src );
        createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
        thresh_callback( 0, 0 );
        waitKey(50);
    }
    cout<<"cars"<<p/100<<endl;
    return(0);
}
/**
 * @function thresh_callback
 */
void thresh_callback(int, void* )
{
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    /// Detect edges using Threshold
    threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
    cv::Mat element5(5,5,CV_8U,cv::Scalar(1));
    dilate(threshold_output,threshold_output,cv::Mat());
    cv::morphologyEx(threshold_output,threshold_output,cv::MORPH_OPEN,element5);
/// Find contours
    cv::  findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    /// Approximate contours to polygons + get bounding rects and circles
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size());
    vector<Point2f>center( contours.size());
    vector<float>radius( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
    }
    /// Draw polygonal contour + bonding rects + circles
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );

    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( 0,0,255);
        drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        rectangle( src1, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
        n++;
        p++;
    }

    /// Show in a window
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    imshow( "Contours",drawing );
    namedWindow( "Contours1", CV_WINDOW_AUTOSIZE );
    imshow( "Contours1",src1);
    namedWindow("contours3",CV_WINDOW_AUTOSIZE);
    imshow("contours3",threshold_output);
    cout<<"cars"<<n<<endl;
}

