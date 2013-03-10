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

	colorMatch(fileNames, images);
	textureMatch(fileNames, images);

	// waitKey(0); // Wait for a keystroke in the window
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
	curs.insertText("COLOR\n---------\n");
	displayHistogramResults(curs, fileNames, locals, globals);

	
	Mat bigImage = manyToOne(images, 10, 4);
	namedWindow("all");
	imshow("all", bigImage);

	return 0;
}

int MainWindow::textureMatch(vector<string> &fileNames, vector<Mat> &images)
{
	int numImages = images.size();
	//will hold processed histograms
	vector<Mat> histograms(numImages);
	vector<Mat> laplacians(numImages);


	//make kernel for laplacian
	float kernel[3][3] = {{1., 1., 1.},
						{1., -8., 1.},
						{1., 1., 1.}};

	// for testing the min and max ranges of the laplacians
	Mat minMax(numImages, 2, CV_32F);

	for (int i = 0; i < numImages; i++)
	{
		// convert image to greyscale
		Mat grey(images[i].size(), CV_32F, Scalar(0));
		for( int j = 0; j < images[i].rows; j++)
			for( int k = 0; k < images[i].cols; k++)
			{
				const Vec3b& pix = images[i].at<Vec3b>(j,k);
				grey.at<float>(j,k) = (pix[0] + pix[1] + pix[2])/3;
			}


		// apply the laplacian kernel
		Mat laplacian(grey.size(),CV_32F);
		for( int j = 0; j < grey.rows; j++)
			for( int k = 0; k < grey.cols; k++)
			{
				laplacian.at<float>(j,k) = grey.at<float>(j,k);

				//skip the fringe elements
				if(j == 0 || k == 0 || 
					j == grey.rows -1 || k == grey.cols -1)
					continue;

				//apply kernel
				laplacian.at<float>(j,k) *= kernel[1][1];
				laplacian.at<float>(j,k) += grey.at<float>(j-1,k-1) * kernel[0][0];
				laplacian.at<float>(j,k) += grey.at<float>(j-1,k) * kernel[0][1];
				laplacian.at<float>(j,k) += grey.at<float>(j-1,k+1) * kernel[0][2];
				laplacian.at<float>(j,k) += grey.at<float>(j,k-1) * kernel[1][0];
				laplacian.at<float>(j,k) += grey.at<float>(j,k+1) * kernel[1][2];
				laplacian.at<float>(j,k) += grey.at<float>(j-1,k-1) * kernel[2][0];
				laplacian.at<float>(j,k) += grey.at<float>(j,k-1) * kernel[2][1];
				laplacian.at<float>(j,k) += grey.at<float>(j+1,k-1) * kernel[2][2];

				// // testing functionality to find the range
				// // for background exclusion
				// if(fabs(laplacian.at<float>(j,k)) < 10)
				// 	laplacian.at<float>(j,k) = 1;
				// else
				// 	laplacian.at<float>(j,k) = 0;

			}


		// // find local and global max TESTING
		// double min =0, max =0;
		// minMaxIdx(laplacian, &min, &max);
		// minMax.at<float>(i,0) = min;
		// minMax.at<float>(i,1) = max;
		// Scalar avg = mean(laplacian);
		// this->ui->textEdit->append(QString("i: %3\tmin: %1\tavg: %4\tmax: %2")
		// 						   .arg(min,5,'f').arg(max).arg(i).arg(avg[0]*100,5,'f'));


		// HISTOGRAM the laplacian
		float histMax = 3600, buckets = 4000, totalPix = 0;
		const float LAPLACE_THRESH = 12;
		// init the histogram and clear it out
		Mat lapHistogram(1, buckets, CV_32F, Scalar(0));
		for (int j=0; j <laplacian.rows; j++)
			for (int k=0; k <laplacian.rows; k++)
			{
				//count the pixels for each bucket
				float pixel = laplacian.at<float>(j,k);

//				//threshold here if you will, but i do not
//				if(fabs(pixel) < LAPLACE_THRESH)
//				    continue;

				// have to take the negative values into account
				int idx = (int) pixel * (buckets/histMax) + buckets/2;
				lapHistogram.at<float>(0,idx) += 1.0;
				totalPix +=1;
			}

		// NORMALIZE the histogram
		float normRatio = 1./totalPix;
		lapHistogram *= normRatio;

		// add the histogram to the vector
		histograms[i] = lapHistogram;
		// keep the laplacian for display
		 laplacian *= 1./200;
		laplacians[i] = laplacian;
	}
	
	// // for testing global maxs for laplacian
	// double min =0, max =0;
	// minMaxIdx(minMax, &min, &max);
	// this->ui->textEdit->append(QString("global min: %1\tglobal max: %2")
	// 						   .arg(min).arg(max));


	// calculate the normalized L1 comparison of the histograms
	double locals[numImages][2][2], globals[2][3];
	calcL1Norm(histograms, locals, globals);

	// display the results
	QTextCursor curs = this->ui->textEdit->textCursor();
	curs.insertText("\n\n\nTEXTURE\n---------\n");
	displayHistogramResults(curs, fileNames, locals, globals);

	// cout<< laplacians[0];
	Mat bigImage = manyToOne(laplacians, 10, 4);
	namedWindow("laplacian");
	imshow("laplacian", bigImage);


	return 0;
}
// 






















