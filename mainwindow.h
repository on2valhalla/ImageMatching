#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//SYS
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

namespace Ui {
class MainWindow;
}

using namespace std;
using namespace cv;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void colorMatch(vector<string> &fileNames, vector<Mat> &images,
					Mat &colorVals);
	void textureMatch(vector<string> &fileNames, vector<Mat> &images,
					Mat &textureVals);
	void comboMatch(vector<string> &fileNames, vector<Mat> &images,
            Mat &colorVals, Mat &textureVals, Mat &comboVals);
	void run();
	
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
