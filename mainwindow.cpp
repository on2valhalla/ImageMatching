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

	// float data[4][3] = {{.5, .1, .2}, {.5, .4, .6}, {.1, .4, .3}, {.2, .6, .3}};
	// Mat test(4, 3, CV_32F, data);

	try2(comboVals);

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


	// QTextCursor curs = this->ui->textEdit->textCursor();
	// curs.insertText("COLOR\n---------\n");
	// displayResults(curs, fileNames, colorVals);

	
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
	// QTextCursor curs = this->ui->textEdit->textCursor();
	// curs.insertText("\n\n\nTEXTURE\n---------\n");
	// displayResults(curs, fileNames, textureVals);

	// display the laplacian images
	Mat bigImage = manyToOne(laplacians, 10, 4);
	namedWindow("laplacian");
	imshow("laplacian", bigImage);

}

void MainWindow::comboMatch(vector<string> &fileNames, vector<Mat> &images,
			Mat &colorVals, Mat &textureVals, Mat &comboVals)
{
	float r = .5, s = 1.0;
	QTextCursor curs = this->ui->textEdit->textCursor();

	for (int i = 0; i < NUM_IMAGES; i ++)
		for (int j = 0; j < NUM_IMAGES; j ++)
		{

			// keep track of all the values
			if(j < i)
			{
				float colorVal = colorVals.at<float>(i,j);
				float textureVal = textureVals.at<float>(i,j);
				float comboVal = (r * colorVal) + ((s - r) * textureVal);
				comboVals.at<float>(i, j) = comboVal;
			}
			else if(j > i)
			{
				float colorVal = colorVals.at<float>(i,j-1);
				float textureVal = textureVals.at<float>(i,j-1);
				float comboVal = (r * colorVal) + ((s - r) * textureVal);
				comboVals.at<float>(i, j-1) = comboVal;
			}
			// cout << i << ", " << j << ":\t" << colorVal << "\t" << textureVal << "\t"
			//     << comboVal << "\t" << comboVals.at<float>(i,j) << endl;
		}

	curs.insertText("\n\n\nCOMBO\n---------\n");
	displayResults(curs, fileNames, comboVals);

}



DendNode* MainWindow::try2(Mat &comboVals)
{
	// convert the combination values to distances
	// and disregard matches to same element 
	// (2 is out of range for min)
	comboVals = 1 - comboVals;
	for (int i = 0; i < comboVals.rows; i++)
		comboVals.at<float>(i,i) = 2;

	// initialize a node on the list for every image
	list<DendNode*> nodes;
	for (int i = 0; i < NUM_IMAGES; i++)
	{
		DendNode *node = new DendNode(i);
		nodes.push_back(node);
	}

	// loop until only one element remains
	while (nodes.size() > 1)
	{
		/* 	
			for every node in the list, check the rest of the list
			for the minimum match... remember to check all the children
			of every node (all indicies are contained in a childIndicies
			list)
		*/
		float min = 2;
		list<DendNode*>::iterator it, jt, mini, minj;
		for (it = nodes.begin(); it != nodes.end(); it++)
			for (jt = nodes.begin(); jt != nodes.end(); jt++)
			{
				float match = (*it)->maxMatch( *jt, comboVals );
				if( match < min)
				{
					min = match;
					mini = it;
					minj = jt;
				}
			}

		// Once the min value and indicies are found, create a new node
		// with the two indicies as children
		DendNode *match = new DendNode(*mini, *minj, min);
        // cout << **mini << "  " << **minj << endl;
        cout<< *match << endl;
		nodes.erase(mini);
		nodes.erase(minj);
		nodes.push_back(match);
	}

	// pop the final node
	DendNode* root = *nodes.begin();
	return root;
}


















// {
// 	//convert to distance
// 	comboVals = 1 - comboVals;

// 	// create a new Mat to play with distances
// 	Mat distances(comboVals.rows, comboVals.cols + 1, CV_32F);
// 	vector<vector<int> *> inds;

//     // cout<<comboVals<<endl;

// 	float records[comboVals.rows][3];
// 	int cnt = 0;
// 	for (int i =0; i < distances.rows; i ++)
// 	{
// 		//push the inds onto the list as lists
// 		vector<int> *tmp = new vector<int>(1,i);
// 		inds.push_back(tmp);
// 		for (int j =0; j < distances.cols; j ++)
// 		{
// 			// make perfect square out of the values
//             if( j < i)
// 				distances.at<float>(i, j) = comboVals.at<float>(i, j);
// 			else if (j == i)
// 				distances.at<float>(i, j) = 2;
// 			else
// 				distances.at<float>(i, j) = comboVals.at<float>(i, j - 1);
// 		}
// 	}

	
//     // cout<<distances<<"\nsize"<<inds.size()<<endl;

// 	while (inds.size() > 1)
// 	{
// 		//find min/max
// 		double min, max;
// 		int minX[2];
// 		minMaxIdx(distances, &min, &max, minX);

// 		// copy everything from one match to the other
// 		vector<int> *left = inds[minX[0]];
// 		cout<<"Joining: ";
// 		for (int i = 0; i < left->size(); i++)
//             cout<<(*left)[i]<<",";
// 		vector<int> *right = inds[minX[1]];
// 		left->insert(left->end(), right->begin(), right->end());
// 		inds.erase(inds.begin() + minX[1]);

// 		cout<<" and ";
// 		for (int i = 0; i < right->size(); i++)
//             cout<<(*right)[i]<<",";
// 		cout<<"at: "<< min<<endl;

// 		distances.create(inds.size(), inds.size(), CV_32F);





// 		// recreate the lookup table for combined lists
// 		for (int i = 0; i < distances.rows; i++)
// 			for (int j = 0; j < distances.cols; j++)
// 			{
// 				// cout<<i<<","<<j<<endl;
// 				if (i == j) // eliminate the crossovers
// 				{
// 					// cout<<"equal"<<endl;
// 					distances.at<float>(i,j) = 2;
// 				}
//                 else if (inds[i]->size() > 1)
// 				{  // list of indicies, find max
// 					float max = 0;
// 					for (int k = 0; k < inds[i]->size(); k++)
// 					{
// 						// cout<<"k: "<< k <<endl;
// 						if (comboVals.at<float>((*inds[i])[k],j) > max)
// 							max = comboVals.at<float>((*inds[i])[k],j);
// 					}
// 					// cout<<"max: "<<max<<endl;
// 					distances.at<float>(i,j) = max;
// 				}
// 				else // single element, update distance
// 				{
// 					distances.at<float>(i,j) = comboVals.at<float>((*inds[i])[0],j);

// 					// cout<<"single: "<<comboVals.at<float>(inds[i][0],j)<<endl;
// 				}

// 			}
// 	    // cout<<distances<<endl;
// 	}
// }





















