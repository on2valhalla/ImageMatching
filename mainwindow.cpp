/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

Main for the Image Matching program

*/

//local
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "dendnode.h"

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
// 


void MainWindow::createDendrogram(Mat &comboVals)
{
	// transition to distance matrix values
//	comboVals = 1 - comboVals;
    cout<<comboVals<<endl;
	// create a new Mat to play with distances
	Mat distances(comboVals.rows, comboVals.cols + 1, CV_32F);
	for (int i =0; i < distances.rows; i ++)
		for (int j =0; j < distances.cols; j ++)
		{
			// so we only have to deal with one set of vals
            if( j < i)
				distances.at<float>(i, j) = comboVals.at<float>(i, j);
			else
				distances.at<float>(i, j) = 2;
		}
    cout<<distances<<endl;

	// to represent the dendrogram
	DendNode *tree = new DendNode();

	for (int i =0; i < comboVals.rows; i ++)
	{
		cout << distances << endl;
		//find min/max
		double min, max;
		int minIdx[2];
		minMaxIdx(distances, &min, &max, minIdx);
		int m1 = minIdx[0];
		int m2 = minIdx[1];
		cout << "("<<m1<<","<<m2<<")"<<endl;

		// remove the match from consideration
		distances.at<float>(m1, m2) = 2;

		if (tree->size() == 0)
		{
			DendNode *child1 = new DendNode(), *child2 = new DendNode();
			child1->index = m1;
			child2->index = m2;
			tree->addChild(child1);
			tree->addChild(child2);
			tree->distance = min;
			for (int j = 0; j < distances.row(m1).cols; j ++)
			{
				// skip any indexes already excluded
				if (distances.at<float>(j, m1) == 2)
					continue;

				// update the values of the first to match the
				// maximums of the tree below
				if(distances.at<float>(j, m1) > distances.at<float>(m2, j))
					distances.at<float>(m2, j) = distances.at<float>(j, m1);
				else
					distances.at<float>(j, m1) = distances.at<float>(m2, j);

				// remove the second match from consideration
				distances.at<float>(m2, j) = 2;
			}
		}
		else
		{
			DendNode *child = new DendNode();
			child->index = m1;
            if(tree->size() == 2)
			{
				DendNode *newTree = new DendNode();
				newTree->addChild(tree);
				tree = newTree;
			}

			// update values
			list<int> indicies = getIndicies(tree);
			while (indicies.size() > 0)
			{
                int idx = indicies.back();
                indicies.pop_back();
				for(int j = 0; j < distances.row(idx).cols; j++)
				{
					// skip any indexes already excluded
					if (distances.at<float>(m1, j) == 2)
						continue;
					if(distances.at<float>(m1, j) > distances.at<float>(idx, j))
						distances.at<float>(idx, j) = distances.at<float>(j, m1);
				}
			}

			tree->distance = min;
			tree->addChild(child);

		}
	}
	cout << tree->toString() << endl;


}



void MainWindow::try2(Mat &comboVals)
{
	//convert to distance
	comboVals = 1 - comboVals;

	// create a new Mat to play with distances
	Mat distances(comboVals.rows, comboVals.cols + 1, CV_32F);
	vector<vector<int> *> inds;

    // cout<<comboVals<<endl;

	float records[comboVals.rows][3];
	int cnt = 0;
	for (int i =0; i < distances.rows; i ++)
	{
		//push the inds onto the list as lists
		vector<int> *tmp = new vector<int>(1,i);
		inds.push_back(tmp);
		for (int j =0; j < distances.cols; j ++)
		{
			// make perfect square out of the values
            if( j < i)
				distances.at<float>(i, j) = comboVals.at<float>(i, j);
			else if (j == i)
				distances.at<float>(i, j) = 2;
			else
				distances.at<float>(i, j) = comboVals.at<float>(i, j - 1);
		}
	}

	
    // cout<<distances<<"\nsize"<<inds.size()<<endl;

	while (inds.size() > 1)
	{
		//find min/max
		double min, max;
		int minX[2];
		minMaxIdx(distances, &min, &max, minX);

		// copy everything from one match to the other
		vector<int> *left = inds[minX[0]];
		cout<<"Joining: ";
		for (int i = 0; i < left->size(); i++)
            cout<<(*left)[i]<<",";
		vector<int> *right = inds[minX[1]];
		left->insert(left->end(), right->begin(), right->end());
		inds.erase(inds.begin() + minX[1]);

		cout<<" and ";
		for (int i = 0; i < right->size(); i++)
            cout<<(*right)[i]<<",";
		cout<<"at: "<< min<<endl;

		distances.create(inds.size(), inds.size(), CV_32F);





		// recreate the lookup table for combined lists
		for (int i = 0; i < distances.rows; i++)
			for (int j = 0; j < distances.cols; j++)
			{
				// cout<<i<<","<<j<<endl;
				if (i == j) // eliminate the crossovers
				{
					// cout<<"equal"<<endl;
					distances.at<float>(i,j) = 2;
				}
                else if (inds[i]->size() > 1)
				{  // list of indicies, find max
					float max = 0;
					for (int k = 0; k < inds[i]->size(); k++)
					{
						// cout<<"k: "<< k <<endl;
						if (comboVals.at<float>((*inds[i])[k],j) > max)
							max = comboVals.at<float>((*inds[i])[k],j);
					}
					// cout<<"max: "<<max<<endl;
					distances.at<float>(i,j) = max;
				}
				else // single element, update distance
				{
					distances.at<float>(i,j) = comboVals.at<float>((*inds[i])[0],j);

					// cout<<"single: "<<comboVals.at<float>(inds[i][0],j)<<endl;
				}

			}
	    // cout<<distances<<endl;
	}
}





















