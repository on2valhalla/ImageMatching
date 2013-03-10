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

	// colorMatch(fileNames, images);
	textureMatch(fileNames, images);

	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);
//	waitKey(0); // Wait for a keystroke in the window
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

	return 0;
}

int MainWindow::textureMatch(vector<string> &fileNames, vector<Mat> &images)
{
	int numImages = images.size();
	//will hold processed histograms
	vector<Mat> histograms(numImages);


	//make kernel for laplacian
	double kData[] = { 1.0, 1.0, 1.0,
					1.0, -8.0, 1.0,
					1.0, 1.0, 1.0 };
	Mat kernel(3, 3, CV_32F, kData);

	// for testing the min and max ranges of the laplacians
	// Mat minMax(numImages, 2, CV_32F);

	
	for (int i = 0; i < numImages; i++)
	{
		// convert image to greyscale
		Mat grey;
		cvtColor(images[i], grey, CV_BGR2GRAY);

		// apply the laplacian kernel
		Mat laplacian(grey.size(), grey.type());
		filter2D(grey, laplacian, CV_32F, kernel);


		// // find local and global max TESTING
		// double min =0, max =0;
		// minMaxIdx(laplacian, &min, &max);
		// minMax.at<float>(i,0) = min;
		// minMax.at<float>(i,1) = max;
		// this->ui->textEdit->append(QString("i: %3\tmin: %1\tmax: %2")
		// 						   .arg(min).arg(max).arg(i));


		// HISTOGRAM the laplacian
		int histMax = 1900;
		int buckets = 50;
		int totalPix = 0;
		// init the histogram and clear it out
		Mat lapHist(1, buckets, CV_32F, Scalar(0));
		for (int j=0; j <laplacian.rows; j++)
			for (int k=0; k <laplacian.rows; k++)
			{
				//count the pixels for each bucket
				float pixel = laplacian.at<float>(j,k);

				//threshold here somehow


				lapHist.at<float>(0,pixel*buckets/histMax) += 1.0;
				totalPix +=1;
			}

		// NORMALIZE the histogram
		float normRatio = 1./totalPix;
		lapHist *= normRatio;

		// add the histogram to the vector
		histograms[i] = lapHist;
	}
	








	// double min =0, max =0;
	// minMaxIdx(minMax, &min, &max);
	// this->ui->textEdit->append(QString("global min: %1\t\tglobal max: %2")
	// 					.arg(min).arg(max));

	// namedWindow("grey");
	// imshow("grey", texImages[0]);

	
}























