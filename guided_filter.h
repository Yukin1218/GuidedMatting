#ifndef GUIDED_FILTER
#define GUIDED_FILTER

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <string>
#include <cmath>

using namespace cv;
using namespace std;

void box_filter(Mat &src, Mat &dst, int r);
void guided_filter_color(Mat &I, Mat &p, Mat &q, int r, double eps);

#endif