/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

A utility class containing useful tools for interacting with images in
QT and OpenCV, as well as providing other functions for Image Matching


*/
#ifndef UTILS_H
#define UTILS_H

//OpenCV
#include <opencv2/core/core.hpp>

//SYS
#include <vector>
#include <string>
#include <iostream>

//local
#include "colorhistogram.h"

using namespace std;
using namespace cv;

//copies many images to one image
//all images must be the same size as the first
Mat manyToOne(vector<Mat> &images, int numRows, int numCols)
{
    Mat sampleImage = images[0];
    int imageWidth = sampleImage.cols;
    int imageHeight = sampleImage.rows;

    int buffer = 5;

    int bigWidth = numRows * (imageHeight + 2*buffer);
    int bigHeight = numCols * (imageWidth + 2*buffer);

    Mat bigImage ( bigWidth, bigHeight, sampleImage.type());

    for (unsigned int i = 0; i < images.size(); i++)
    {
        int row = i / numCols;
        int col = i % numCols;
        int x = col * imageWidth + 2*col*buffer + buffer;
        int y = row * imageHeight + 2*row*buffer + buffer;

        Mat roi(bigImage, Rect(x, y, imageWidth, imageHeight));

        images[i].copyTo(roi);
    }

    return bigImage;
}

//Gets number of images from a specified directory
void getImages(vector<Mat> &images, string dir, int count)
{
    for (int i = 1; i <= count; i++)
    {
        stringstream sstm;
        if( i >= 10)
            sstm << dir << "i" << i << ".ppm";
        else
            sstm << dir << "i0" << i << ".ppm";

        string fileName = sstm.str();
        Mat image = imread(fileName, 1);
        images.push_back(image);
    }
}

//Gets number of images from a specified directory
void getImages(IplImage *images[], string dir, int count)
{
    for (int i = 1; i <= count; i++)
    {
        stringstream sstm;
        if( i >= 10)
            sstm << dir << "i" << i << ".ppm";
        else
            sstm << dir << "i0" << i << ".ppm";

        string fileName = sstm.str();
        images[i-1] = cvLoadImage(fileName.c_str(), 1);
    }
}

// take a list of images and give back histograms
//
void getHistograms(const vector<Mat> &images, vector<Mat> &histograms, int buckets = 10, int blackThresh = 25, int whiteThresh = 25)
{
    const int histSize[] = {buckets, buckets, buckets};


    for (unsigned int i = 0; i < images.size(); i++)
    {
        Mat image = images[i];
        Mat histogram(3, histSize, CV_32F, Scalar(0));

        CV_Assert(image.type() == CV_8UC3);
        int numPixels = 0;
        for (int j = 0; j < image.rows; j++)
            for (int k = 0; k < image.cols; k++)
            {
                const Vec3b& pix = image.at<Vec3b>(i,j);
                //threshold black values by skipping them
                if(pix[0] + pix[1] + pix[2] < blackThresh*3)
                    continue;
                //threshold white values by skipping them
                if(pix[0] + pix[1] + pix[2] > (255 - whiteThresh)*3 )
                    continue;

                histogram.at<float>(pix[0]*buckets/256, pix[1]*buckets/256, pix[2]*buckets/256) += 1.0;
                numPixels +=1;
            }


        //Get a 3d iterator that returns a Mat that is
        //a slice (plane) of the original nary matrix
        cv::Mat plane;
        const cv::Mat* hists[] = { &histogram, 0 };
        cv::NAryMatIterator itN(hists, &plane, 1);

        //Normalize the histogram
        double normRatio = 1./numPixels;
        for(unsigned int p = 0; p < itN.nplanes; p++, ++itN)
            itN.planes[0] *= normRatio;

        histograms.push_back(histogram);
    }

}




#endif // UTILS_H
