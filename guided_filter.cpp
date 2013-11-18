/*
 * Implementation of the guided filter.
 */
#include "guided_filter.h"

/*
 * dst(x, y)=sum(sum(src(x-r:x+r,y-r:y+r)))
 * both src and dst are CV_32FC1
 */
void box_filter(Mat &src, Mat &dst_ori, int r)
{
	//cout<<"box!\n";
	const int hei = src.rows;
	const int wid = src.cols;
	Mat dst;
	dst_ori.copyTo(dst);
	src.copyTo(dst);
	dst.setTo(0);

	Mat cum;
	src.copyTo(cum);
	// cumulative sum over Y axis
	for(int row = 1; row < hei; ++row) { 
		float *cum_prev_row = cum.ptr<float>(row - 1);
		float *cum_curr_row = cum.ptr<float>(row);
		for(int col = 0; col < wid; ++col) {
			cum_curr_row[col] += cum_prev_row[col];
		}
	}
	// difference over Y axis
	for(int row = 0; row < r + 1; ++row) {
		float *dst_row = dst.ptr<float>(row);
		float *cum_row = cum.ptr<float>(row + r);
		for(int col = 0; col < wid; ++col) {
			dst_row[col] = cum_row[col];
		}
	}
	for(int row = r + 1; row < hei - r; ++row) {
		float *dst_row = dst.ptr<float>(row);
		float *cum_row1 = cum.ptr<float>(row + r);
		float *cum_row2 = cum.ptr<float>(row - r -1);
		for(int col = 0; col < wid; ++col) {
			dst_row[col] = cum_row1[col] - cum_row2[col];
		}
	}
	for(int row = hei - r; row < hei; ++row) {
		float *dst_row = dst.ptr<float>(row);
		float *cum_last_row = cum.ptr<float>(hei - 1);
		float *cum_row = cum.ptr<float>(row - r - 1);
		for(int col = 0; col < wid; ++col) {
			dst_row[col] = cum_last_row[col] - cum_row[col];
		}
	}

	dst.copyTo(cum);
	// cumulative sum over X axis
	for(int row = 0; row < hei; ++row) {
		float *cum_row = cum.ptr<float>(row);
		for(int col = 1; col < wid; ++col) {
			cum_row[col] += cum_row[col - 1];
		}
	}
	// difference over X axis
	for(int row = 0; row < hei; ++row) {
		float *dst_row = dst.ptr<float>(row);
		float *cum_row = cum.ptr<float>(row);
		for(int col = 0; col < r + 1; ++col) {
			dst_row[col] = cum_row[col + r];
		}
		for(int col = r + 1; col < wid - r; ++col) {
			dst_row[col] = cum_row[col + r] - cum_row[col - r - 1];
		}
		for(int col = wid - r; col < wid; ++col) {
			dst_row[col] = cum_row[wid - 1] - cum_row[col - r - 1];
		}
	}
	dst.copyTo(dst_ori);
	dst.release();
	cum.release();
	return;
}

/*
 * Implementation of the guided color filter.
 * I is the colorful guide image(CV_32FC3)
 * p, q are the input and output(both CV_32FC1).
 */
void guided_filter_color(Mat &I, Mat &p, Mat &q, int r, double eps)
{
	p.copyTo(q);
	const int hei = p.rows;
	const int wid = p.cols;

	Mat N;
	Mat tmp = Mat(hei, wid, CV_32FC1);
	tmp.setTo(1);
	box_filter(tmp, N, r);
	///////////////////////////////////////////
	//normalize(N, N, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", N);
	//waitKey(0);
	///////////////////////////////////////////

	vector<Mat> rgb;
	split(I, rgb); // bgr
	//namedWindow("hehe");
	//imshow("hehe", rgb[0]);
	//waitKey(0);
	Mat mean_I_b, mean_I_g, mean_I_r;
	box_filter(rgb[0], mean_I_b, r);
	box_filter(rgb[1], mean_I_g, r);
	box_filter(rgb[2], mean_I_r, r);
	mean_I_b = mean_I_b / N;
	mean_I_g = mean_I_g / N;
	mean_I_r = mean_I_r / N;
	///////////////////////////////////////////////
	//Mat show;
	//normalize(mean_I_b, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	//////////////////////////////////////////////////
	Mat mean_p;
	box_filter(p, mean_p, r);
	mean_p = mean_p / N;
	//cout<<*max_element(mean_p.begin<float>(), mean_p.end<float>())<<endl;
	/////////////////////////////////////////////
	//Mat show;
	//normalize(mean_p, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	////////////////////////////////////////////////
	Mat mean_Ip_b, mean_Ip_g, mean_Ip_r;
	mean_Ip_b = rgb[0].mul(p);
	mean_Ip_g = rgb[1].mul(p);
	mean_Ip_r = rgb[2].mul(p);
	box_filter(mean_Ip_b, mean_Ip_b, r);
	box_filter(mean_Ip_g, mean_Ip_g, r);
	box_filter(mean_Ip_r, mean_Ip_r, r);
	mean_Ip_b = mean_Ip_b / N;
	mean_Ip_g = mean_Ip_g / N;
	mean_Ip_r = mean_Ip_r / N;
	///////////////////////////////////////////////
	//Mat show;
	//normalize(mean_Ip_b, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	//////////////////////////////////////////////////

	// co-variance of (I, p) in each local patch
	Mat cov_Ip_b, cov_Ip_g, cov_Ip_r;
	cov_Ip_b = mean_Ip_b - mean_I_b.mul(mean_p);
	cov_Ip_g = mean_Ip_g - mean_I_g.mul(mean_p);
	cov_Ip_r = mean_Ip_r - mean_I_r.mul(mean_p);
	/////////////////////////////////////////////////
	//cout<<*min_element(cov_Ip_g.begin<float>(), cov_Ip_g.end<float>())<<endl;
	//Mat show;
	//normalize(-1*cov_Ip_b, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	////////////////////////////////////////////////////

	// variance of I in each local patch: the matrix Sigma in Eqn(14).
	// variance in each local patch is a 3x3 symmetric matrix:
	// Sigma = rr, rg, rb
	//         rg, gg, gb
	//         rb, gb, bb
	Mat var_I_rr, var_I_rg, var_I_rb, var_I_gg, var_I_gb, var_I_bb;
	tmp = rgb[2].mul(rgb[2]);
	box_filter(tmp, var_I_rr, r);
	tmp = rgb[2].mul(rgb[1]);
	box_filter(tmp, var_I_rg, r);
	tmp = rgb[2].mul(rgb[0]);
	box_filter(tmp, var_I_rb, r);
	tmp = rgb[1].mul(rgb[1]);
	box_filter(tmp, var_I_gg, r);
	tmp = rgb[1].mul(rgb[0]);
	box_filter(tmp, var_I_gb, r);
	tmp = rgb[0].mul(rgb[0]);
	box_filter(tmp, var_I_bb, r);
	var_I_rr = var_I_rr / N - mean_I_r.mul(mean_I_r);
	var_I_rg = var_I_rg / N - mean_I_r.mul(mean_I_g);
	var_I_rb = var_I_rb / N - mean_I_r.mul(mean_I_b);
	var_I_gg = var_I_gg / N - mean_I_g.mul(mean_I_g);
	var_I_gb = var_I_gb / N - mean_I_g.mul(mean_I_b);
	var_I_bb = var_I_bb / N - mean_I_b.mul(mean_I_b);
	///////////////////////////////////////////////
	//cout<<*max_element(var_I_rr.begin<float>(), var_I_rr.end<float>())<<endl;
	//Mat show;
	//normalize(var_I_rr, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	//////////////////////////////////////////////////

	Mat a;
	I.copyTo(a);
	a.setTo(0);
	Mat Sigma = Mat(3, 3, CV_32FC1);
	Mat cov_Ip = Mat(1, 3, CV_32FC1);
	Mat aa = Mat(1, 3, CV_32FC1);
	for(int y = 0; y < hei; ++y) {
		float *var_I_rr_row = var_I_rr.ptr<float>(y);
		float *var_I_rg_row = var_I_rg.ptr<float>(y);
		float *var_I_rb_row = var_I_rb.ptr<float>(y);
		float *var_I_gg_row = var_I_gg.ptr<float>(y);
		float *var_I_gb_row = var_I_gb.ptr<float>(y);
		float *var_I_bb_row = var_I_bb.ptr<float>(y);
		float *cov_Ip_r_row = cov_Ip_r.ptr<float>(y);
		float *cov_Ip_g_row = cov_Ip_g.ptr<float>(y);
		float *cov_Ip_b_row = cov_Ip_b.ptr<float>(y);
		for(int x = 0; x < wid; ++x) {
			Sigma.at<float>(0, 0) = var_I_rr_row[x] + eps;
			Sigma.at<float>(0, 1) = var_I_rg_row[x];
			Sigma.at<float>(0, 2) = var_I_rb_row[x];
			Sigma.at<float>(1, 0) = var_I_rg_row[x];
			Sigma.at<float>(1, 1) = var_I_gg_row[x] + eps;
			Sigma.at<float>(1, 2) = var_I_gb_row[x];
			Sigma.at<float>(2, 0) = var_I_rb_row[x];
			Sigma.at<float>(2, 1) = var_I_gb_row[x];
			Sigma.at<float>(2, 2) = var_I_bb_row[x] + eps;
			cov_Ip.at<float>(0, 0) = cov_Ip_r_row[x];
			cov_Ip.at<float>(0, 1) = cov_Ip_g_row[x];
			cov_Ip.at<float>(0, 2) = cov_Ip_b_row[x];
			aa = cov_Ip * Sigma.inv();
			//cout<<aa.size()<<endl; // TODO
			a.at<Vec3f>(y, x)[0] = aa.at<float>(0, 0);
			a.at<Vec3f>(y, x)[1] = aa.at<float>(0, 1);
			a.at<Vec3f>(y, x)[2] = aa.at<float>(0, 2);
		}
	}
	// Eqn(15)
	vector<Mat> av;
	split(a, av);
	///////////////////////////////////////////////
	//Mat show;
	//cout<<*min_element(av[2].begin<float>(), av[2].end<float>())<<endl;
	//normalize(av[2], show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	//////////////////////////////////////////////////
	Mat b;
	b = mean_p - av[0].mul(mean_I_r) - av[1].mul(mean_I_g) - av[2].mul(mean_I_b);
	///////////////////////////////////////////////
	//Mat show;
	//cout<<*max_element(b.begin<float>(), b.end<float>())<<endl;
	//normalize(b, show, 1, 0, NORM_MINMAX);
	//namedWindow("tmp");
	//imshow("tmp", show);
	//waitKey(0);
	//show.release();
	//////////////////////////////////////////////////
	// Eqn(16)
	q.setTo(0);
	box_filter(av[0], tmp, r);
	q = q + tmp.mul(rgb[2]);
	box_filter(av[1], tmp, r);
	q = q + tmp.mul(rgb[1]);
	box_filter(av[2], tmp, r);
	q = q + tmp.mul(rgb[0]);
	box_filter(b, tmp, r);
	q = q + tmp;
	q = q / N;
	N.release();
	tmp.release();
	mean_I_b.release();
	mean_I_g.release();
	mean_I_r.release();
	mean_p.release();
	mean_Ip_b.release();
	mean_Ip_g.release();
	mean_Ip_r.release();
	cov_Ip_b.release();
	cov_Ip_g.release();
	cov_Ip_r.release();
	var_I_rr.release();
	var_I_rg.release();
	var_I_rb.release();
	var_I_gg.release();
	var_I_gb.release();
	var_I_bb.release();
	a.release();
	aa.release();
	b.release();
	return;
}