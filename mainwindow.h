#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

//SYS
#include <iostream>
#include <string>
#include <vector>
#include <cmath>


using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void colorMatch(vector<string> &fileNames, vector<Mat> &images,
					double colorVals[NUM_IMAGES][NUM_IMAGES]);
	void textureMatch(vector<string> &fileNames, vector<Mat> &images,
					double textureVals[NUM_IMAGES][NUM_IMAGES]);
	void comboMatch(vector<string> &fileNames, vector<Mat> &images
			double colorVals[NUM_IMAGES][NUM_IMAGES], double textureVals[NUM_IMAGES][NUM_IMAGES])
	void run();
	
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
