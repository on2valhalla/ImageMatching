/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

Main for the Image Matching program

*/

//local
#include "utils.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//Retrieve the images from the filesystem
	vector<string> fileNames;
	vector<Mat> images;
	getImages(images, fileNames, IMG_DIR, NUM_IMAGES);

	colorMatch(fileNames, images);
	textureMatch(fileNames, images);
}

MainWindow::~MainWindow()
{
	delete ui;
}

int MainWindow::colorMatch(vector<string> &fileNames, vector<Mat> &images)
{
	const int BUCKETS = 10, B_THRESH = 30, W_THRESH = 0;


	//get their respective histograms
	vector<Mat> histograms;
	getHistograms(images, histograms, BUCKETS, B_THRESH, W_THRESH);

	//will hold the normalized L1 comparison data
	double locals[NUM_IMAGES][2][2];
	double globals[2][3];
	calcL1Norm(histograms, locals, globals);


	QTextCursor curs = this->ui->textEdit->textCursor();
	displayHistogramResults(curs, fileNames, locals, globals);
    


	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);
//	waitKey(0); // Wait for a keystroke in the window

	return 0;
}

int MainWindow::textureMatch(vector<string> &fileNames, vector<Mat> &images)
{

}
