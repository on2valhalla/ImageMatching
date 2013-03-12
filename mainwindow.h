#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCursor>
#include <QTextDocumentWriter>


//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

//SYS
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include "dendnode.h"

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
	void comboMatch(vector<string> &fileNames,
            Mat &colorVals, Mat &textureVals, Mat &comboVals);
	DendNode* linkage( Mat & comboVals, int linkageType = 0);
    int drawDendrogram(QTextCursor &texCurs, QTextCursor &imgCurs, vector<string> &imgNames, 
    	DendNode *tree, int totWidth = 40, int startWidth =100);
	void run();
	
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
