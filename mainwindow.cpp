/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

Main for the Image Matching program

*/

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

//SYS
#include <iostream>
#include <string>
#include <vector>

//local
#include "utils.h"
#include "colorhistogram.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;

static const string IMG_DIR = "../../../../ImageMatching/img/";

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	init();
}

MainWindow::~MainWindow()
{
	delete ui;
}

int MainWindow::init()
{
    const int NUM_IMAGES = 40, BUCKETS = 16, B_THRESH = 30, W_THRESH = 0;

    //Retrieve the images from the filesystem
	vector<string> fileNames;
    vector<Mat> images;
    getImages(images, fileNames, IMG_DIR, NUM_IMAGES);

	//get their respective histograms
	vector<Mat> histograms;
    getHistograms(images, histograms, BUCKETS, B_THRESH, W_THRESH);

	//will hold the normalized L1 comparison data
    Mat norms(NUM_IMAGES, NUM_IMAGES , CV_32F);
    for (int i = 0; i < NUM_IMAGES; i++)
        for (int j = 0; j < NUM_IMAGES; j++)
		{
			// get the l1 norm of the two histograms
			double normVal = norm(histograms[i],histograms[j], NORM_L1);

			// map the value to [0,1] (1 is the same image)
            normVal = 1 - (normVal/2);
            norms.at<float>(i,j) = normVal;
            //hack to make min max work
            if( i == j)
                norms.at<float>(i,j) = .1;
        }


    //get display images
    vector<QImage> qImages(NUM_IMAGES);
    for (int i = 0; i< NUM_IMAGES; i++)
        qImages[i] = QImage(fileNames[i].c_str());

	QTextCursor curs = this->ui->textEdit->textCursor();
    curs.insertText("Color Histogram Matches");
	curs.insertTable(40,3);
	for (int i = 0; i< NUM_IMAGES; i++)
    {
        curs.insertImage(qImages[i]);
        curs.insertText("\n" + QString(fileNames[i].c_str()).section("/",-1));
		curs.movePosition(QTextCursor::NextCell);

        //find max/min match  (idx: 0/1)
        double *vals[2];
        int idxs[2][2];
        minMaxIdx(norms.col(i), vals[1], vals[0], idxs[1], idxs[0]);

        curs.insertImage(qImages[idxs[0][0]]);
        curs.insertText("\n" + QString(fileNames[idxs[0][0]].c_str()).section("/",-1)
                + QString("\n%1").arg(*vals[0],4,'f',5));
		curs.movePosition(QTextCursor::NextCell);

        curs.insertImage(qImages[idxs[1][0]]);
        curs.insertText("\n" + QString(fileNames[idxs[0][0]].c_str()).section("/",-1)
                + QString("\n%1").arg(*vals[1],4,'f',5));
        curs.movePosition(QTextCursor::NextCell);
	}


	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);
//	waitKey(0); // Wait for a keystroke in the window

	return 0;
}
