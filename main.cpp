/* 
 * Test the guided filter implementation.
 * Apply on matting.
 */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <fstream>
#include "guided_filter.h"
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	Mat I = imread("I.png");
	I.convertTo(I, CV_32FC3);
	I = I / 255;
	Mat p = imread("p.png", CV_LOAD_IMAGE_GRAYSCALE);
	p.convertTo(p, CV_32FC1);
	p = p / 255;
	int r = 60;
	double eps = 0.0000001;

	Mat q;
	guided_filter_color(I, p, q, r, eps);
	//normalize(q, q, 1, 0, NORM_MINMAX);
	namedWindow("q");
	imshow("q", q);
	waitKey(0);

	p.release();
	I.release();
	return 0;
}