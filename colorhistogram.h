/*------------------------------------------------------------------------------------------*\
   This file contains material supporting chapter 4 of the cookbook:
   Computer Vision Programming using the OpenCV Library.
   by Robert Laganiere, Packt Publishing, 2011.

   This program is free software; permission is hereby granted to use, copy, modify,
   and distribute this source code, or portions thereof, for any purpose, without fee,
   subject to the restriction that the copyright notice may not be removed
   or altered from any source or altered source distribution.
   The software is released on an as-is basis and without any warranties of any kind.
   In particular, the software is not guaranteed to be fault-tolerant or free from failure.
   The author disclaims all warranties with regard to this software, any use,
   and any consequent failure, is purely the responsibility of the user.

   Copyright (C) 2010-2011 Robert Laganiere, www.laganiere.name
\*------------------------------------------------------------------------------------------*/

/*
    This class wraps easy methods for dealing with histograms around OpenCV2 functions

    It also provides a colorReduce function for use before sending an image for
    processing. This code has been modified, but retains the original copyright above.

    Modified by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)
*/


#if !defined COLHISTOGRAM
#define COLHISTOGRAM

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>

class ColorHistogram {

  private:

    int histSize[3];
    float hranges[2];
    const float* ranges[3];
    int channels[3];

  public:

    ColorHistogram() {

        // Prepare arguments for a color histogram
        histSize[0]= histSize[1]= histSize[2]= 256;
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        ranges[0]= hranges; // all channels have the same range
        ranges[1]= hranges;
        ranges[2]= hranges;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;
    }

    // Computes the histogram.
    cv::MatND getHistogram(const cv::Mat &image)
    {

        cv::MatND hist;

        // BGR color histogram
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;

        // Compute histogram
        cv::calcHist(&image,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            3,			// it is a 3D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the histogram.
    cv::SparseMat getSparseHistogram(const cv::Mat &image)
    {

        cv::SparseMat hist(3,histSize,CV_32F);

        // BGR color histogram
        hranges[0]= 0.0;    // BRG range
        hranges[1]= 255.0;
        channels[0]= 0;		// the three channels
        channels[1]= 1;
        channels[2]= 2;

        // Compute histogram
        cv::calcHist(&image,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            3,			// it is a 3D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 2D ab histogram.
    // BGR source image is converted to Lab
    cv::MatND getabHistogram(const cv::Mat &image)
    {

        cv::MatND hist;

        // Convert to Lab color space
        cv::Mat lab;
        cv::cvtColor(image, lab, CV_BGR2Lab);

        // Prepare arguments for a 2D color histogram
        hranges[0]= -128.0;
        hranges[1]= 127.0;
        channels[0]= 1; // the two channels used are ab
        channels[1]= 2;

        // Compute histogram
        cv::calcHist(&lab,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            2,			// it is a 2D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 1D Hue histogram with a mask.
    // BGR source image is converted to HSV
    cv::MatND getHueHistogram(const cv::Mat &image)
    {

        cv::MatND hist;

        // Convert to Lab color space
        cv::Mat hue;
        cv::cvtColor(image, hue, CV_BGR2HSV);

        // Prepare arguments for a 1D hue histogram
        hranges[0]= 0.0;
        hranges[1]= 180.0;
        channels[0]= 0; // the hue channel

        // Compute histogram
        cv::calcHist(&hue,
            1,			// histogram of 1 image only
            channels,	// the channel used
            cv::Mat(),	// no mask is used
            hist,		// the resulting histogram
            1,			// it is a 1D histogram
            histSize,	// number of bins
            ranges		// pixel value range
        );

        return hist;
    }

    // Computes the 1D Hue histogram with a mask.
    // BGR source image is converted to HSV
    cv::Mat getHSHistogram(const cv::Mat &image)
    {
        cv::Mat hsv;

        cv::cvtColor(image, hsv, CV_BGR2HSV);

        // Quantize the hue to 30 levels
        // and the saturation to 32 levels
        int hbins = 30, sbins = 32;
        int histSize[] = {hbins, sbins};
        // hue varies from 0 to 179, see cvtColor
        float hranges[] = { 0, 180 };
        // saturation varies from 0 (black-gray-white) to
        // 255 (pure spectrum color)
        float sranges[] = { 0, 256 };
        const float* ranges[] = { hranges, sranges };
        cv::MatND hist;
        // we compute the histogram from the 0-th and 1-st channels
        int channels[] = {0, 1};

        cv::calcHist( &hsv, 1, channels, cv::Mat(), // do not use mask
                 hist, 2, histSize, ranges,
                 true, // the histogram is uniform
                 false );
        double maxVal=0;
        cv::minMaxLoc(hist, 0, &maxVal, 0, 0);

        int scale = 10;
        cv::Mat histImg = cv::Mat::zeros(sbins*scale, hbins*10, CV_8UC3);

        for( int h = 0; h < hbins; h++ )
            for( int s = 0; s < sbins; s++ )
            {
                float binVal = hist.at<float>(h, s);
                int intensity = cvRound(binVal*255/maxVal);
                cv::rectangle( histImg, cv::Point(h*scale, s*scale),
                            cv::Point( (h+1)*scale - 1, (s+1)*scale - 1),
                            cv::Scalar::all(intensity),
                            CV_FILLED );
            }
        return histImg;
    }

    cv::Mat colorReduce(const cv::Mat &image, int div=64)
    {

      int n= static_cast<int>(log(static_cast<double>(div))/log(2.0));
      // mask used to round the pixel value
      uchar mask= 0xFF<<n; // e.g. for div=16, mask= 0xF0

      cv::Mat_<cv::Vec3b>::const_iterator it= image.begin<cv::Vec3b>();
      cv::Mat_<cv::Vec3b>::const_iterator itend= image.end<cv::Vec3b>();

      // Set output image (always 1-channel)
      cv::Mat result(image.rows,image.cols,image.type());
      cv::Mat_<cv::Vec3b>::iterator itr= result.begin<cv::Vec3b>();

      for ( ; it!= itend; ++it, ++itr) {

        (*itr)[0]= ((*it)[0]&mask) + div/2;
        (*itr)[1]= ((*it)[1]&mask) + div/2;
        (*itr)[2]= ((*it)[2]&mask) + div/2;
      }

      return result;
    }

    // Returns an image of a histogram of the image in BGR
    cv::Mat getHistogramImage(const cv::Mat &image)
    {
        /// Separate the image in 3 places ( B, G and R )
        std::vector<cv::Mat> bgr_planes;
        cv::split( image, bgr_planes );

        /// Establish the number of bins
        int histSize = 256;

        /// Set the ranges ( for B,G,R) )
        float range[] = { 0, 256 } ;
        const float* histRange = { range };

        bool uniform = true;
        bool accumulate = false;

        cv::Mat b_hist, g_hist, r_hist;

        /// Compute the histograms:
        cv::calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
        cv::calcHist( &bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

        // Draw the histograms for B, G and R
        int hist_w = 512; int hist_h = 400;
        int bin_w = cvRound( (double) hist_w/histSize );

        cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

        /// Normalize the result to [ 0, histImage.rows ]
        cv::normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );


        /// Draw for each channel
        for( int i = 1; i < histSize; i++ )
        {
          cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                           cv::Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
          cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                           cv::Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                           cv::Scalar( 0, 255, 0), 2, 8, 0  );
          cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                           cv::Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                           cv::Scalar( 0, 0, 255), 2, 8, 0  );
        }


        return histImage;
    }
};


#endif
