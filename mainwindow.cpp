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
    int numImages = 40, buckets = 25;

        //Retrieve the images from the filesystem
        vector<Mat> images;
        getImages(images, IMG_DIR, numImages);

        //get their respective histograms
        vector<Mat> histograms;
        getHistograms(images, histograms, buckets);



        //will hold the normalized L1 comparison data
        double norms[numImages][numImages];

        // organized by image, (max/min), (value/idx)
        int localMaxMin[numImages][2][2];
        // organized by (max/min), (value/idx1/idx2)
        int globalMaxMinPair[2][3];
        for (int i = 1; i <= numImages; ++i)
        {
            //set initial values
            localMaxMin[i][0][0] = 0;
            localMaxMin[i][1][0] = 2;
            globalMaxMinPair[0][0] = 0;
            globalMaxMinPair[1][0] = 2;
        }




        for (int i = 0; i < numImages; ++i)
        {
            for (int j = 0; j < numImages; ++j)
            {
                // get the l1 norm of the two histograms
                double normVal = norm(histograms[i],histograms[j], NORM_L1);

                // map the value to [0,1] (1 is the same image)
                normVal = 1 - (normVal/2);

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

                norms[i][j] = normVal;
            }
        }

        QTextCursor curs = this->ui->textEdit->textCursor();
        curs.insertTable(40,3);
        for (int i = 0; i< numImages; i++)
        {
            Mat image = images[i];
            cvtColor(image, image, CV_BGR2RGB);
            QImage qImage = QImage((const unsigned char*)image.data,image.cols,
                                   image.rows, image.step, QImage::Format_RGB888);
            curs.insertImage(qImage);
            curs.movePosition(QTextCursor::NextCell);
            curs.movePosition(QTextCursor::NextCell);
            curs.movePosition(QTextCursor::NextCell);
        }



//        cout << norms[0];
//        vector<Mat> triples;
//        for (int i = 0; i < 2; i ++)
//        {
//            cout << i;
//            vector<Mat> tmp;
//            tmp.push_back(images[i]);
//            tmp.push_back(images[localMaxMin[i][0][1]]);
//            tmp.push_back(images[localMaxMin[i][1][1]]);
//            Mat combine = manyToOne(tmp, 1, 3);
//            triples.push_back(combine);
//        }

        // Mat bigTriples = manyToOne(triples, 10, 4);
        // namedWindow("triples");
        // imshow("triples", bigTriples);


        // Mat bigImage = manyToOne(images, 10, 4);
        // namedWindow("all");
        // imshow("all", bigImage);
        // waitKey(0); // Wait for a keystroke in the window

	return 0;
}
