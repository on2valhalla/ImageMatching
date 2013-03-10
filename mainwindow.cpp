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
    const int NUM_IMAGES = 40, BUCKETS = 16, B_THRESH = 30, W_THRESH = 30;

    //Retrieve the images from the filesystem
	vector<string> fileNames;
    vector<Mat> images;
    getImages(images, fileNames, IMG_DIR, NUM_IMAGES);

	//get their respective histograms
	vector<Mat> histograms;
    getHistograms(images, histograms, BUCKETS, B_THRESH, W_THRESH);



	//will hold the normalized L1 comparison data
	double norms[NUM_IMAGES][NUM_IMAGES];

	// organized by image, (max/min), (value/idx)
    double localMaxMin[NUM_IMAGES][2][2];
    for (int i = 0; i < NUM_IMAGES; i++)
	{
		//set initial values
		localMaxMin[i][0][0] = 0;
        localMaxMin[i][1][0] = 2;
	}

    // organized by (max/min), (value/idx1/idx2)
    double globalMaxMinPair[2][3];
    globalMaxMinPair[0][0] = 0;
    globalMaxMinPair[1][0] = 2;



    for (int i = 0; i < NUM_IMAGES; i++)
	{
        for (int j = 0; j < NUM_IMAGES; j++)
		{
			// get the l1 norm of the two histograms
			double normVal = norm(histograms[i],histograms[j], NORM_L1);

			// map the value to [0,1] (1 is the same image)
            normVal = 1 - (normVal/2);
            norms[i][j] = normVal;

			if (i == j)
				continue;

			if (normVal > localMaxMin[i][0][0])
			{
				localMaxMin[i][0][0] = normVal;
				localMaxMin[i][0][1] = j;
			}
			if (normVal < localMaxMin[i][1][0])
			{
				localMaxMin[i][1][0] = normVal;
				localMaxMin[i][1][1] = j;
			}
			if (normVal > globalMaxMinPair[0][0])
			{
				globalMaxMinPair[0][0] = normVal;
				globalMaxMinPair[0][1] = i;
				globalMaxMinPair[0][2] = j;
			}
			if (normVal < globalMaxMinPair[1][0])
			{
				globalMaxMinPair[1][0] = normVal;
				globalMaxMinPair[1][1] = i;
				globalMaxMinPair[1][2] = j;
            }
		}
	}


	QTextCursor curs = this->ui->textEdit->textCursor();
    curs.insertText("Color Histogram Matches");
	curs.insertTable(40,3);
	for (int i = 0; i< NUM_IMAGES; i++)
	{
		QImage qImage = QImage(fileNames[i].c_str());
		curs.insertImage(qImage);
        curs.insertText("\n" + QString(fileNames[i].c_str()).section("/",-1));
		curs.movePosition(QTextCursor::NextCell);

        int maxIdx = (int) localMaxMin[i][0][1];
        qImage = QImage(fileNames[maxIdx].c_str());
		curs.insertImage(qImage);
        curs.insertText("\n" + QString(fileNames[maxIdx].c_str()).section("/",-1));
		curs.movePosition(QTextCursor::NextCell);

        int minIdx = (int) localMaxMin[i][1][1];
        qImage = QImage(fileNames[minIdx].c_str());
		curs.insertImage(qImage);
        curs.insertText("\n" + QString(fileNames[minIdx].c_str()).section("/",-1));
		curs.movePosition(QTextCursor::NextCell);

	}



	// cout << norms[0];
	// vector<Mat> triples;
	// for (int i = 0; i < 2; i ++)
	// {
	//    cout << i;
	//    vector<Mat> tmp;
	//    tmp.push_back(images[i]);
	//    tmp.push_back(images[localMaxMin[i][0][1]]);
	//    tmp.push_back(images[localMaxMin[i][1][1]]);
	//    Mat combine = manyToOne(tmp, 1, 3);
	//    triples.push_back(combine);
	// }

	// Mat bigTriples = manyToOne(triples, 10, 4);
	// namedWindow("triples");
	// imshow("triples", bigTriples);


	// Mat bigImage = manyToOne(images, 10, 4);
	// namedWindow("all");
	// imshow("all", bigImage);
	// waitKey(0); // Wait for a keystroke in the window

	return 0;
}
