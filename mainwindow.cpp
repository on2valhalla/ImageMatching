/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

Main for the Image Matching program

*/

//local
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"

using namespace util;


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::run()
{
	//Retrieve the images from the filesystem
	vector<string> fileNames;
	vector<Mat> images;
	getImages(images, fileNames, IMG_DIR, NUM_IMAGES);

	// -1 for y val because you dont want to include self match
	Mat colorVals(NUM_IMAGES, NUM_IMAGES -1, CV_32F);
	Mat textureVals(NUM_IMAGES, NUM_IMAGES -1, CV_32F);
	Mat comboVals(NUM_IMAGES, NUM_IMAGES -1, CV_32F);

	colorMatch(fileNames, images, colorVals);
	textureMatch(fileNames, images, textureVals);
	comboMatch(fileNames, images, colorVals, textureVals, comboVals);

	// waitKey(0); // Wait for a keystroke in the window
}

void MainWindow::colorMatch(vector<string> &fileNames, vector<Mat> &images,
					Mat &colorVals)
{
	const int BUCKETS = 10, B_THRESH = 30, W_THRESH = 0;


	//get their respective histograms
	vector<Mat> histograms;
	getHistograms(images, histograms, BUCKETS, B_THRESH, W_THRESH);

	//will hold the normalized L1 comparison data
	double locals[NUM_IMAGES][2][2];
	double globals[2][3];
	calcL1Norm(histograms, locals, globals, colorVals);


	QTextCursor curs = this->ui->textEdit->textCursor();
	curs.insertText("COLOR\n---------\n");
	displayResults(curs, fileNames, colorVals);

	
	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);

}

void MainWindow::textureMatch(vector<string> &fileNames, vector<Mat> &images,
						Mat &textureVals)
{
	int NUM_IMAGES = images.size();
	//will hold processed histograms
	vector<Mat> histograms(NUM_IMAGES);
	vector<Mat> laplacians(NUM_IMAGES);



	// for testing the min and max ranges of the laplacians
	// Mat minMax(NUM_IMAGES, 2, CV_32F);

	for (int i = 0; i < NUM_IMAGES; i++)
	{
		// convert image to greyscale
		Mat grey;
		bgrToGrey(images[i], grey);

		// apply the laplacian kernel
		Mat laplacian;
		applyLaplacian(grey, laplacian);

		// //  FOR TESTING
		// // find local and global max
		// double min =0, max =0;
		// minMaxIdx(laplacian, &min, &max);
		// minMax.at<float>(i,0) = min;
		// minMax.at<float>(i,1) = max;
		// Scalar avg = mean(laplacian);
		// this->ui->textEdit->append(QString("i: %3\tmin: %1\tavg: %4\tmax: %2")
		// 						   .arg(min,5,'f').arg(max).arg(i).arg(avg[0]*100,5,'f'));

		// HISTOGRAM the laplacian
		Mat histogram;
		getLaplaceHistogram(laplacian, histogram);

		// add the histogram to the vector
		histograms[i] = histogram;
		// keep the laplacian for display
		laplacian *= 1./200;
		laplacians[i] = laplacian;
	}
	
	// //  FOR TESTING
	// // for testing global maxs for laplacian
	// double min =0, max =0;
	// minMaxIdx(minMax, &min, &max);
	// this->ui->textEdit->append(QString("global min: %1\tglobal max: %2")
	// 						   .arg(min).arg(max));


	// calculate the normalized L1 comparison of the histograms
	double locals[NUM_IMAGES][2][2], globals[2][3];
	calcL1Norm(histograms, locals, globals, textureVals);

	// display the results
	QTextCursor curs = this->ui->textEdit->textCursor();
	curs.insertText("\n\n\nTEXTURE\n---------\n");
	displayResults(curs, fileNames, textureVals);

	// display the laplacian images
	Mat bigImage = manyToOne(laplacians, 10, 4);
	namedWindow("laplacian");
	imshow("laplacian", bigImage);

}

void MainWindow::comboMatch(vector<string> &fileNames, vector<Mat> &images,
			Mat &colorVals, Mat &textureVals, Mat &comboVals)
{
	float r = .7, s = 1.0;
	QTextCursor curs = this->ui->textEdit->textCursor();

	for (int i = 0; i < NUM_IMAGES; i ++)
		for (int j = 0; j < NUM_IMAGES; j ++)
		{
			float colorVal = colorVals.at<float>(i,j);
			float textureVal = textureVals.at<float>(i,j);
			float comboVal = (r * colorVal) + ((s - r) * textureVal);


			// keep track of all the values
			if(j < i)
				comboVals.at<float>(i, j) = comboVal;
			else if(j > i)
				comboVals.at<float>(i, j-1) = comboVal;
			// cout << i << ", " << j << ":\t" << colorVal << "\t" << textureVal << "\t" 
			// 	<< comboVal << "\t" << comboVals.at<float>(i,j) << endl;
		}

	curs.insertText("\n\n\nTEXTURE\n---------\n");
	displayResults(curs, fileNames, comboVals);

}
// 






















