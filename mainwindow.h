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
    int colorMatch(vector<string> &fileNames, vector<Mat> &images);
    int textureMatch(vector<string> &fileNames, vector<Mat> &images);
    void run();
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
