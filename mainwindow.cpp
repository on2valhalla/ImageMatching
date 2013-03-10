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
	const int NUM_IMAGES = 40, BUCKETS = 10, B_THRESH = 30, W_THRESH = 0;

	//Retrieve the images from the filesystem
	vector<string> fileNames;
	vector<Mat> images;
	getImages(images, fileNames, IMG_DIR, NUM_IMAGES);

	//get their respective histograms
	vector<Mat> histograms;
	getHistograms(images, histograms, BUCKETS, B_THRESH, W_THRESH);

	//will hold the normalized L1 comparison data
	int maxIdx[NUM_IMAGES], minIdx[NUM_IMAGES];
	double localMax[NUM_IMAGES], localMin[NUM_IMAGES];
	int globalIdx[2][2];
	double globalMax = 0, globalMin = 10000;
	for (int i = 0; i < NUM_IMAGES; i++)
	{
		localMax[i] = 0;
		localMin[i] = 10000;
		for (int j = 0; j < NUM_IMAGES; j++)
		{
			// get the l1 norm of the two histograms
			double normVal = norm(histograms[i],histograms[j], NORM_L1);

			// map the value to [0,1] (1 is the same image)
			normVal = 1 - (normVal/2.);
			if(normVal < 0)
				normVal = 0;

			//hack to make min max work
			if( i == j)
				continue;
			if (normVal > localMax[i])
			{
				localMax[i] = normVal;
				maxIdx[i] = j;
			}
			if (normVal < localMin[i])
			{
				localMin[i] = normVal;
				minIdx[i] = j;
			}
			if (normVal > globalMax)
			{
				globalMax = normVal;
				globalIdx[0][0] = i;
				globalIdx[0][1] = j;
			}
			if (normVal < globalMin)
			{
				globalMin = normVal;
                globalIdx[1][0] = i;
                globalIdx[1][1] = j;
			}
		}
	}


	//get display images
	vector<QImage> qImages(NUM_IMAGES);
	for (int i = 0; i< NUM_IMAGES; i++)
		qImages[i] = QImage(fileNames[i].c_str());

	QTextCursor curs = this->ui->textEdit->textCursor();
    curs.insertText("Color Histogram Local Matches (Max/Min)");
	curs.insertTable(40,3);
	for (int i = 0; i< NUM_IMAGES; i++)
	{
		curs.insertImage(qImages[i]);
		curs.insertText("\n" + QString(fileNames[i].c_str()).section("/",-1));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[maxIdx[i]]);
		curs.insertText("\n" + QString(fileNames[maxIdx[i]].c_str()).section("/",-1)
				+ QString("\n%1").arg(localMax[i],4,'f',5));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[minIdx[i]]);
		curs.insertText("\n" + QString(fileNames[minIdx[i]].c_str()).section("/",-1)
                + QString("\n%1").arg(localMin[i],4,'f',5));
        curs.movePosition(QTextCursor::NextCell);
	}


    curs.movePosition(QTextCursor::End);
    curs.insertText("\n\nColor Histogram Global Matches (Max/Min)");
    curs.insertTable(2,2);

    curs.insertImage(qImages[globalIdx[0][0]]);
    curs.insertText("\n" + QString(fileNames[globalIdx[0][0]].c_str()).section("/",-1)
            + QString("\n%1").arg(globalMax));
    curs.movePosition(QTextCursor::NextCell);
    curs.insertImage(qImages[globalIdx[0][1]]);
    curs.insertText("\n" + QString(fileNames[globalIdx[0][1]].c_str()).section("/",-1)
            + QString("\n%1").arg(globalMax));
    curs.movePosition(QTextCursor::NextCell);

    curs.insertImage(qImages[globalIdx[1][0]]);
    curs.insertText("\n" + QString(fileNames[globalIdx[1][0]].c_str()).section("/",-1)
            + QString("\n%1").arg(globalMin));
    curs.movePosition(QTextCursor::NextCell);
    curs.insertImage(qImages[globalIdx[1][1]]);
    curs.insertText("\n" + QString(fileNames[globalIdx[1][1]].c_str()).section("/",-1)
            + QString("\n%1").arg(globalMin));
    curs.movePosition(QTextCursor::NextCell);


	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);
//	waitKey(0); // Wait for a keystroke in the window

	return 0;
}
