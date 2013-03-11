/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

A utility class containing useful tools for interacting with images in
QT and OpenCV, as well as providing other functions for Image Matching


*/
#ifndef UTILS_H
#define UTILS_H
//QT
#include <QTextCursor>

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//SYS
#include <vector>
#include <string>
#include <iostream>



namespace util
{
	using namespace std;
	using namespace cv;


	static const string IMG_DIR = "../../../../ImageMatching/img/";
	static const int NUM_IMAGES = 40;

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
	void getImages(vector<Mat> &images, vector<string> &fileNames, string dir, int count)
	{
		for (int i = 1; i <= count; i++)
		{
			stringstream sstm;
			if( i >= 10)
				sstm << dir << "i" << i << ".ppm";
			else
				sstm << dir << "i0" << i << ".ppm";

			string fileName = sstm.str();
			fileNames.push_back(fileName);
			Mat image = imread(fileName, 1);
			images.push_back(image);
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
					const Vec3b& pix = image.at<Vec3b>(j,k);
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

	void calcL1Norm(vector<Mat> &histograms, double locals[NUM_IMAGES][2][2], 
				double globals[2][3], Mat &allVals)
	{
		globals[0][0] = 10000; //global min val
		globals[1][0] = 0; // global max val
		for (int i = 0; i < NUM_IMAGES; i++)
		{
			locals[i][0][0] = 10000; //local min val
			locals[i][1][0] = 0; //local max val
			for (int j = 0; j < NUM_IMAGES; j++)
			{
				// get the l1 norm of the two histograms
				double normVal = norm(histograms[i],histograms[j], NORM_L1);

				// map the value to [0,1] (1 is the same image)
				normVal = 1 - (normVal/2.);
				if(normVal < 0)
					normVal *= -1;

				// keep track of all the values
				if(j < i)
					allVals.at<float>(i, j) = (float) normVal;
				else if(j > i)
					allVals.at<float>(i, j-1) = (float) normVal;
				// cout << allVals.at<float>(i,j) <<endl;

				//hack to make min max work
				if( i == j)
					continue;
				if (normVal < locals[i][0][0])
				{
					locals[i][0][0] = normVal;
					locals[i][0][1] = j;
				}
				if (normVal > locals[i][1][0])
				{
					locals[i][1][0] = normVal;
					locals[i][1][1] = j;
				}
				if (normVal < globals[0][0])
				{
					globals[0][0] = normVal;
					globals[0][1] = i;
					globals[0][2] = j;
				}
				if (normVal > globals[1][0])
				{
					globals[1][0] = normVal;
					globals[1][1] = i;
					globals[1][2] = j;
				}
			}
		}
	}

	void displayHistogramResults(QTextCursor &curs, vector<string> &fileNames, 
									double locals[][2][2], double globals[2][3])
	{

		//get display images
		vector<QImage> qImages(NUM_IMAGES);
		for (int i = 0; i< NUM_IMAGES; i++)
			qImages[i] = QImage(fileNames[i].c_str());


		curs.insertText("Histogram Local Matches (Max/Min)");
		curs.insertTable(40,3);
		for (int i = 0; i< NUM_IMAGES; i++)
		{
			curs.insertImage(qImages[i]);
			curs.insertText("\n" + QString(fileNames[i].c_str()).section("/",-1));
			curs.movePosition(QTextCursor::NextCell);

			double min = locals[i][0][0];
			int minIdx = (int)locals[i][0][1];
			double max = locals[i][1][0];
			int maxIdx = (int)locals[i][1][1];
			curs.insertImage(qImages[maxIdx]);
			curs.insertText("\n" + QString(fileNames[maxIdx].c_str()).section("/",-1)
					+ QString("\n%1").arg(max,4,'f',5));
			curs.movePosition(QTextCursor::NextCell);

			curs.insertImage(qImages[minIdx]);
			curs.insertText("\n" + QString(fileNames[minIdx].c_str()).section("/",-1)
					+ QString("\n%1").arg(min,4,'f',5));
			curs.movePosition(QTextCursor::NextCell);
		}


		curs.movePosition(QTextCursor::End);
		curs.insertText("\n\nHistogram Global Matches (Max/Min)");
		curs.insertTable(2,2);


		double min = globals[0][0];
		int minIdx1 = (int)globals[0][1];
		int minIdx2 = (int)globals[0][2];
		double max = globals[1][0];
		int maxIdx1 = (int)globals[1][1];
		int maxIdx2 = (int)globals[1][2];
		
		curs.insertImage(qImages[maxIdx1]);
		curs.insertText("\n" + QString(fileNames[maxIdx1].c_str()).section("/",-1)
				+ QString("\n%1").arg(max));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[maxIdx2]);
		curs.insertText("\n" + QString(fileNames[maxIdx2].c_str()).section("/",-1)
				+ QString("\n%1").arg(max));
		curs.movePosition(QTextCursor::NextCell);


		curs.insertImage(qImages[minIdx1]);
		curs.insertText("\n" + QString(fileNames[minIdx1].c_str()).section("/",-1)
				+ QString("\n%1").arg(min));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[minIdx2]);
		curs.insertText("\n" + QString(fileNames[minIdx2].c_str()).section("/",-1)
				+ QString("\n%1").arg(min));
		curs.movePosition(QTextCursor::NextCell);
	}

	// converts image to grey
	void bgrToGrey(const Mat &image, Mat &grey)
	{
		// recreate if necessary
		grey.create(image.size(), CV_32F);


		for( int j = 0; j < image.rows; j++)
			for( int k = 0; k < image.cols; k++)
			{
				const Vec3b& pix = image.at<Vec3b>(j,k);
				grey.at<float>(j,k) = (pix[0] + pix[1] + pix[2])/3;
			}
	}

	// input an image in greyscale
	void applyLaplacian(const Mat &grey, Mat &laplacian)
	{
		//make kernel for laplacian
		float kernel[3][3] = {{1., 1., 1.},
							{1., -8., 1.},
							{1., 1., 1.}};

		//create the Mat if necessary
		laplacian.create(grey.size(), CV_32F);

		for( int j = 0; j < grey.rows; j++)
			for( int k = 0; k < grey.cols; k++)
			{
				laplacian.at<float>(j,k) = grey.at<float>(j,k);

				//skip the edge elements
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
	}

	void getLaplaceHistogram(const Mat &laplacian, Mat &histogram, 
		float buckets = 4000, float lapAbsMax = 1800, float laplaceThresh = 12)
	{
		// init the histogram and clear it out
		histogram = Mat(1, buckets, CV_32F, Scalar(0));

		int totalPix = 0;
		for (int j=0; j <laplacian.rows; j++)
			for (int k=0; k <laplacian.rows; k++)
			{
				//count the pixels for each bucket
				float pixel = laplacian.at<float>(j,k);

				// //threshold here if you will, but i do not
				// if(fabs(pixel) < laplaceThresh)
				//     continue;

				// have to take the negative values into account
				int idx = (int) pixel * buckets/(2*lapAbsMax) + buckets/2;
				histogram.at<float>(0,idx) += 1.0;
				totalPix +=1;
			}

		// NORMALIZE the histogram
		float normRatio = 1./totalPix;
		histogram *= normRatio;
	}


	void displayResults(QTextCursor &curs, vector<string> &fileNames, 
									Mat &values)
	{

		//get display images
		vector<QImage> qImages(NUM_IMAGES);
		for (int i = 0; i< NUM_IMAGES; i++)
			qImages[i] = QImage(fileNames[i].c_str());


		curs.insertText("Local Matches (Max/Min)");
		curs.insertTable(40,3);
		for (int i = 0; i< NUM_IMAGES; i++)
		{
			curs.insertImage(qImages[i]);
			curs.insertText("\n" + QString(fileNames[i].c_str()).section("/",-1));
			curs.movePosition(QTextCursor::NextCell);

			//find min/max
			double min, max;
			int minIdx[2], maxIdx[2];
			minMaxIdx(values.row(i), &min, &max, minIdx, maxIdx);
			if( minIdx[1] >= i)
				minIdx[1]++;
			if( maxIdx[1] >= i)
				maxIdx[1]++;

			cout << i <<" " << values.row(i).cols <<" " << maxIdx[1] <<" " << minIdx[1] << endl;

			curs.insertImage(qImages[maxIdx[1]]);
			curs.insertText("\n" + QString(fileNames[maxIdx[1]].c_str()).section("/",-1)
					+ QString("\n%1").arg(max,4,'f',5));
			curs.movePosition(QTextCursor::NextCell);

			curs.insertImage(qImages[minIdx[1]]);
			curs.insertText("\n" + QString(fileNames[minIdx[1]].c_str()).section("/",-1)
					+ QString("\n%1").arg(min,4,'f',5));
			curs.movePosition(QTextCursor::NextCell);
		}


		curs.movePosition(QTextCursor::End);
		curs.insertText("\n\nGlobal Matches (Max/Min)");
		curs.insertTable(2,2);


		//find min/max
		double min, max;
		int minIdx[2], maxIdx[2];
		minMaxIdx(values, &min, &max, minIdx, maxIdx);
		if(minIdx[1] >= minIdx[0])
			minIdx[1]++;
		if(maxIdx[1] >= maxIdx[0])
			maxIdx[1]++;
		
		cout << maxIdx[0] <<" " << maxIdx[1] <<" " << minIdx[0] <<" " << minIdx[1] <<" " << endl;

		curs.insertImage(qImages[maxIdx[0]]);
		curs.insertText("\n" + QString(fileNames[maxIdx[0]].c_str()).section("/",-1)
				+ QString("\n%1").arg(max));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[maxIdx[1]]);
		curs.insertText("\n" + QString(fileNames[maxIdx[1]].c_str()).section("/",-1)
				+ QString("\n%1").arg(max));
		curs.movePosition(QTextCursor::NextCell);


		curs.insertImage(qImages[minIdx[0]]);
		curs.insertText("\n" + QString(fileNames[minIdx[0]].c_str()).section("/",-1)
				+ QString("\n%1").arg(min));
		curs.movePosition(QTextCursor::NextCell);

		curs.insertImage(qImages[minIdx[1]]);
		curs.insertText("\n" + QString(fileNames[minIdx[1]].c_str()).section("/",-1)
				+ QString("\n%1").arg(min));
		curs.movePosition(QTextCursor::NextCell);
	}
}

#endif // UTILS_H
