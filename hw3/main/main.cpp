#include "image.h"
#include "utilities.h"
#include <strings.h>
#include <string.h>

//MY INCLUDES
#include <fstream>
#include "hsi.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

#define MAXLEN 256
#define MAXROI 1024

int main (int argc, char** argv)
{
	// Image related vars
	image src, tgt;
	string infile, outfile, pch, strtemp;
	ifstream fp;
	int count;
	bool ovlap;

	// OpenCV
	Mat tgt_cv;

	//attempts to open parameters file.
	fp.open(argv[1],fstream::in);
	if(!fp.is_open()) {
		cout << "\nCant open parameters file: " << argv[1] << endl;
		return -1;
	}

	// Kernels
	vector<vector<int> > sobel3x = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};
	vector<vector<int> > sobel3y = {
		{-1, -2, -1},
		{ 0,  0,  0},
		{ 1,  2,  1}
	};
	vector<vector<int> > sobel5x = {
		{ -5,  -4, 0,  4,  5},
		{ -8, -10, 0, 10,  8},
		{-10, -20, 0, 20, 10},
		{ -8, -10, 0, 10,  8},
		{ -5,  -4, 0,  4,  5}
	};
	vector<vector<int> > sobel5y = {
		{-5,  -8, -10,  -8, -5},
		{-4, -10, -20, -10, -4},
		{ 0,   0,   0,   0,  0},
		{ 4,  10,  20,  10,  4},
		{ 5,   8,  10,   8,  5}
	};

	//loop that read every line in parameters file.
	pair<int, int> start, size;
	vector<vector<int> > rois_vec;
	vector<int> roi_vec; 
	while(fp) {
		
		//checks if commented out.
		if(fp.peek() == '#') {
			getline(fp, strtemp);
			if(fp.eof())
				break;
			continue;
		}

		// //gets src image filename.
		fp >> infile;
		if(infile[0] == '#') {
			getline(fp, strtemp);
			if(fp.eof())
				break;
			continue;
		}
		
		//reads output filename.
		fp >> outfile;

		//reads number of processes on image, max is MAXROI.
		fp >> count;
		if(count > MAXROI) {
			cout << "\nMax Number of ROIs/Processes is " << MAXROI << ". Cannot Process Image: " << infile << endl;
			continue;
		}

		//reads then copies image and runs loop for up to 3 processes.
		if(!src.read(infile)) {
			getline(fp,strtemp);
			continue;
		}

		//copies source to target and sets filename.
		tgt = src;
		tgt = outfile;
		tgt_cv = imread(infile, -1);
		bool cv_flag = false;

		if(fp.eof()) break;
		rois_vec.clear();
		for(int i = 0; i < count; ++i) {
			
			//reads process name and ROI.
			pch.clear();
			fp >> pch;
			fp >> start.first >> start.second;
			fp >> size.first >> size.second;
			roi_vec.clear();
			roi_vec.push_back(start.first);
			roi_vec.push_back(start.second);
			roi_vec.push_back(size.first);
			roi_vec.push_back(size.second);

			// Checks for roi overlap
			ovlap = utilities::roi_overlap(rois_vec, roi_vec);
			if(!ovlap)
				rois_vec.push_back(roi_vec);

			// hist equal opencv
			if(!strncasecmp(pch.c_str(),"ocvotsuhist",MAXLEN)) {
				// Local vars
				Mat img_roi, img_roi_temp, img_temp, hist;
				Rect cv_roi = Rect(start.first, start.second, size.first, size.second);
				int pixel_val, bin_val;
				vector<Mat> channels;

				// Checks overlap
				if(!ovlap){
					// Creates roi and temp roi
					img_roi = tgt_cv(cv_roi);
					tgt_cv.copyTo(img_temp); 
					img_roi_temp = img_temp(cv_roi);
					
					// Thresholds by OTSU into temp image
					threshold(img_roi_temp, img_roi_temp, 0, 255, CV_THRESH_OTSU);

					// Find all pixels in foreground
					for(int i = start.second; i < start.second+size.second; i++){
						for(int j = start.first; j < start.first+size.first; j++){
							bin_val = img_roi_temp.at<unsigned char>(i, j);
							if(bin_val == 255){
								pixel_val = tgt_cv.at<unsigned char>(i, j);
								hist.push_back(pixel_val);
							}
						}
					}

					// Runs historgam equal
					hist.convertTo(hist, CV_8UC1);
					split(hist, channels);
					equalizeHist(channels[0], channels[0]);
					merge(channels, hist);

					// Adds pixels back to original img in roi
					for(int i = start.second, count=0; i < start.second+size.second; i++){
						for(int j = start.first; j < start.first+size.first; j++){
							bin_val = img_roi_temp.at<unsigned char>(i, j);
							if(bin_val == 255){
								pixel_val = hist.at<unsigned char>(count);
								count++;
								img_roi.at<unsigned char>(i, j) = pixel_val;
							}
						}
					}

					// Sets flag so it wil save
					cv_flag = true;
				}
			}

			// hist equal opencv
			else if(!strncasecmp(pch.c_str(),"ocvotsu",MAXLEN)) {
				// Local vars
				Mat img_roi, img_roi_temp;
				vector<Mat> channels;
				Rect cv_roi = Rect(start.first, start.second, size.first, size.second);

				// Checks overlap
				if(!ovlap){
					// Creates roi
					img_roi = tgt_cv(cv_roi);

					if(tgt_cv.channels() > 1){
						img_roi_temp = tgt_cv(cv_roi);
						cvtColor(img_roi, img_roi_temp, CV_BGR2GRAY);
						threshold(img_roi_temp, img_roi_temp, 0, 255, CV_THRESH_OTSU);
						cvtColor(img_roi_temp, img_roi, CV_GRAY2BGR);
					}
					else{
						threshold(img_roi, img_roi, 0, 255, CV_THRESH_OTSU);
					}

					cv_flag = true;
				}
			}

			// hist equal opencv
			else if(!strncasecmp(pch.c_str(),"ocvhist",MAXLEN)) {
				// Local vars
				Mat edges, img_roi, img_roi_temp;
				vector<Mat> channels;
				Rect cv_roi = Rect(start.first, start.second, size.first, size.second);

				// Checks overlap
				if(!ovlap){
					// Creates roi and temp roi
					img_roi = tgt_cv(cv_roi);
					img_roi_temp = tgt_cv(cv_roi);

					// Runs hist equal on roi
					// Color
					if(tgt_cv.channels() > 1){
						cvtColor(img_roi, img_roi_temp, COLOR_BGR2HSV);
						split(img_roi_temp, channels);
						equalizeHist(channels[2], channels[2]);
						merge(channels, img_roi_temp);
						cvtColor(img_roi_temp, img_roi, COLOR_HSV2BGR);
					}
					// Gray scale
					else {
						split(img_roi, channels);
						equalizeHist(channels[0], channels[0]);
						merge(channels, img_roi);
					}

					cv_flag = true;
				}
			}

			// canny 3x3 gradient thresh
			else if(!strncasecmp(pch.c_str(),"ocvcanny",MAXLEN)) {
				// Local vars
				Mat edges, img_roi, img_roi_temp;
				Rect cv_roi = Rect(start.first, start.second, size.first, size.second);
				int kernel_size;

				fp >> kernel_size;

				// Checks overlap
				if(!ovlap){
					// Creates roi and temp roi
					img_roi = tgt_cv(cv_roi);
					img_roi_temp = tgt_cv(cv_roi);

					// Converts roi to grayscale
					if(tgt_cv.channels() > 1)
						cvtColor(img_roi_temp, img_roi_temp, CV_BGR2GRAY);
					
					// Gets edges
					if(tgt_cv.channels() > 1){
						Canny(img_roi_temp, edges, 100, 300, kernel_size);
						cvtColor(edges, img_roi, CV_GRAY2BGR);
					}
					else{
						Canny(img_roi, img_roi, 100, 300, kernel_size);
					}
					cv_flag = true;
				}
			}

			// Sobel 3x3 gradient thresh OpenCV
			else if(!strncasecmp(pch.c_str(),"ocvsobel",MAXLEN)) {
				// Local vars
				Mat gx, gy, img_roi, img_roi_temp;
				Rect cv_roi = Rect(start.first, start.second, size.first, size.second);
				int kernel_size;

				fp >> kernel_size;

				// Checks overlap
				if(!ovlap){
					// Creates roi and temp roi
					img_roi = tgt_cv(cv_roi);
					img_roi_temp = tgt_cv(cv_roi);

					// Converts roi to grayscale
					cvtColor(img_roi_temp, img_roi_temp, CV_BGR2GRAY);
					cvtColor(img_roi_temp, img_roi, CV_GRAY2BGR);
					
					// Gets dx, dy gradients
					Sobel(img_roi, gx, CV_32F, 1, 0, kernel_size);
					Sobel(img_roi, gy, CV_32F, 0, 1, kernel_size);
					convertScaleAbs(gx, gx);
					convertScaleAbs(gy, gy);

					// Combines dc and dy
					addWeighted(gx, 0.5, gy, 0.5, 0, img_roi);
					cv_flag = true;
				}
			}

			// Sobel 3x3 gradient thresh
			else if(!strncasecmp(pch.c_str(),"sobelgb",MAXLEN)) {
				
				image tgtd = src, tgtg = src;
				vector<vector<int> > kernelx, kernely;
				int thresh_grad, ws = 3, kernel_size;
				fp >> kernel_size >> thresh_grad;

				if(kernel_size == 3){
					kernelx = sobel3x;
					kernely = sobel3y;
				}
				else{
					kernelx = sobel5x;
					kernely = sobel5y;
				}

				
				// If roi overlap is false
				if(!ovlap){
					utilities::gradient2d(src, tgtg, tgtd, start, size, kernelx, kernely);
					utilities::binarizegs(tgtg, tgt, start, size, thresh_grad, ws);
				}
			}

			// Sobel 3x3 direction thresh
			else if(!strncasecmp(pch.c_str(),"sobeldb",MAXLEN)) {
				
				image tgtd = src, tgtg = src;
				vector<vector<int> > kernelx, kernely;
				int thresh_deg, ws = 3, kernel_size;
				fp >> kernel_size >> thresh_deg;

				if(kernel_size == 3){
					kernelx = sobel3x;
					kernely = sobel3y;
				}
				else{
					kernelx = sobel5x;
					kernely = sobel5y;
				}
				
				// If roi overlap is false
				if(!ovlap){
					utilities::gradient2d(src, tgtg, tgtd, start, size, kernelx, kernely);
					utilities::binarizedeg(tgtd, tgt, start, size, thresh_deg, ws);
				}
			}

			// Sobel 3x3 gradient direction thresh
			else if(!strncasecmp(pch.c_str(),"sobelgbdb",MAXLEN)) {
				
				image tgtd = src, tgtg = src;
				vector<vector<int> > kernelx, kernely;
				int thresh_deg, thresh_grad, ws = 3, kernel_size;
				fp >> kernel_size >> thresh_grad >> thresh_deg;

				if(kernel_size == 3){
					kernelx = sobel3x;
					kernely = sobel3y;
				}
				else{
					kernelx = sobel5x;
					kernely = sobel5y;
				}
				
				// If roi overlap is false
				if(!ovlap){
					utilities::gradient2d(src, tgtg, tgtd, start, size, kernelx, kernely);
					utilities::binarizegsdeg(tgtg, tgtd, tgt, start, size, thresh_grad, thresh_deg, ws);
				}
			}

			// Sobel
			else if(!strncasecmp(pch.c_str(),"sobel",MAXLEN)) {
				
				image tgtg = src;
				vector<vector<int> > kernelx, kernely;
				int kernel_size;
				fp >> kernel_size;

				if(kernel_size == 3){
					kernelx = sobel3x;
					kernely = sobel3y;
				}
				else{
					kernelx = sobel5x;
					kernely = sobel5y;
				}
				
				// If roi overlap is false
				if(!ovlap)
					utilities::gradient2d(src, tgt, tgtg, start, size, kernelx, kernely);
			}

			//grayscale binarization/threshold process.
			else if(!strncasecmp(pch.c_str(),"bin",MAXLEN)) {
				
				int thresh1, thresh2;
				fp >> thresh1 >> thresh2;
				
				// If roi overlap is false
				if(!ovlap)
					utilities::roiBinarizeRange(src, tgt, start, size, thresh1, thresh2);
			}

			//color binarization/threshold process.
			else if (!strncasecmp(pch.c_str(),"binc",MAXLEN)) {
				
				// Vars
				int tc, colors[3];

				//caps data from parameters file.
				fp >> tc;
				fp >> colors[0] >> colors[1] >> colors[2];

				// If roi overlap is false
				if(!ovlap)
					utilities::roiBinarizeColor(src, tgt, tc, colors, start, size);
			}

			//1D smoothing process.
			else if(!strncasecmp(pch.c_str(),"1Dsmooth",MAXLEN)) {
				int ws;

				//collects data from parameters file.
				fp >> ws;

				//makes sure ws > 3 and odd.
				if(ws % 2 == 0 || ws < 3) {
					cout << "\nWindow size must be an odd number 3 or greater for smothing. 2DSmooth will not be processed on " << infile << endl;
					continue;
				}
				
				if(!ovlap)
					utilities::roiSmooth2DAdaptive(src, tgt, ws, start, size);
			}

			//2D smoothing process.
			else if(!strncasecmp(pch.c_str(),"2Dsmooth",MAXLEN)) {
				int ws;

				//collects data from parameters file.
				fp >> ws;

				//makes sure ws > 3 and odd.
				if(ws % 2 == 0 || ws < 3) {
					cout << "\nWindow size must be an odd number 3 or greater for smothing. 2DSmooth will not be processed on " << infile << endl;
					continue;
				}
				
				if(!ovlap)
					utilities::roiSmooth2DAdaptive(src, tgt, ws, start, size);
			}

			// Optimal Thresholding grayscale binarized
			else if(!strncasecmp(pch.c_str(),"othgs",MAXLEN)) {
				// Stuff
				int limit, thresh_opt;

				// Gets difference limit
				// fp >> limit;
				limit = 5;

				// If no overlap
				if(!ovlap){
					thresh_opt = utilities::optimalThreshGS(src, start, size, limit);
					utilities::threshGS(src, tgt, start, size, thresh_opt);
					// cout << "thresh_opt = " << thresh_opt << endl;
				}
			}

			// Optimal Thresholding background
			else if(!strncasecmp(pch.c_str(),"othbr",MAXLEN)) {
				// Stuff
				int limit, thresh_opt;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				// fp >> limit;
				limit = 5;

				// If no overlap
				if(!ovlap){
					// cout << "DEBUG 2\n";
					thresh_opt = utilities::optimalThreshGS(src, start, size, limit);
					utilities::threshGSbr(src, tgt, start, size, thresh_opt);
					// cout << "thresh_opt = " << thresh_opt << endl;
				}
			}

			// Optimal Thresholding forground
			else if(!strncasecmp(pch.c_str(),"othfg",MAXLEN)) {
				// Stuff
				int limit, thresh_opt;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				// fp >> limit;
				limit = 5;

				// If no overlap
				if(!ovlap){
					// cout << "DEBUG 2\n";
					thresh_opt = utilities::optimalThreshGS(src, start, size, limit);
					utilities::threshGSfg(src, tgt, start, size, thresh_opt);
					// cout << "thresh_opt = " << thresh_opt << endl;
				}
			}

			// Histogram Stretching grayscale [a,b]
			else if(!strncasecmp(pch.c_str(),"histgs",MAXLEN)) {
				// Stuff
				pair<int, int> ab;
				int hist_before[256] = {0}, hist_after[256] = {0};
				string stemp(to_string(count));
				string hist_fname = outfile + "-Histogram-" + stemp + ".pgm";

				// cout << "DEBUG 1\n";

				// Gets difference limit
				fp >> ab.first >> ab.second;

				// If no overlap
				if(!ovlap){

					utilities::histStretchGS(src, tgt, start, size, ab);
					utilities::histCreate(src, hist_before, start, size);
					utilities::histCreate(tgt, hist_after, start, size);
					utilities::histSave(hist_before, hist_after, hist_fname);
				}
			}

			// Histogram Stretching grayscale bilinear [a,b] and [c,d]
			else if(!strncasecmp(pch.c_str(),"histgsbl",MAXLEN)) {
				// Stuff
				pair<int, int> ab, cd;
				int hist_before[256] = {0}, hist_after[256] = {0};
				string stemp(to_string(count));
				string hist_fname = outfile + "-Histogram-" + stemp + ".pgm";

				// cout << "DEBUG 1\n";

				// Gets difference limit
				fp >> ab.first >> ab.second;
				fp >> cd.first >> cd.second;

				// If no overlap
				if(!ovlap){

					utilities::histStretchGS(src, tgt, start, size, ab, cd);
					utilities::histCreate(src, hist_before, start, size);
					utilities::histCreate(tgt, hist_after, start, size);
					utilities::histSave(hist_before, hist_after, hist_fname);
				}
			}

			// Histogram background/foreground stretching
			else if(!strncasecmp(pch.c_str(),"histgsoth",MAXLEN)) {
				// Stuff
				pair<int, int> ab_back, ab_forg;
				int hist_before_back[256] = {0}, hist_after_back[256] = {0};
				int hist_before_forg[256] = {0}, hist_after_forg[256] = {0};
				int thresh_opt;
				string stemp(to_string(count));
				string hist_fname_back = outfile + "-Histogram-" + stemp + "_background.pgm";
				string hist_fname_forg = outfile + "-Histogram-" + stemp + "_forground.pgm";
				string hist_fname_back_hs = outfile + "-hs-" + stemp + "_background.pgm";
				string hist_fname_forg_hs = outfile + "-hs-" + stemp + "_forground.pgm";
				string hist_fname_back_nhs = outfile + "-background-" + stemp + ".pgm";
				string hist_fname_forg_nhs = outfile + "-forground-" + stemp + ".pgm";
				image back_img, forg_img;
				
				back_img = src, forg_img = src;
				back_img = hist_fname_back_nhs, forg_img = hist_fname_forg_nhs;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				// fp >> ab.first >> ab.second;

				// If no overlap
				if(!ovlap){
					// Get optimal threshold and separate background/foreground
					thresh_opt = utilities::optimalThreshGS(src, start, size);
					vector<vector<int> > back_vec = utilities::threshGSbr(src, back_img, start, size, thresh_opt);
					vector<vector<int> > forg_vec = utilities::threshGSfg(src, forg_img, start, size, thresh_opt);
					
					// Finds min and max of both
					ab_back.first = 1000, ab_back.second = 0;
					ab_forg.first = 1000, ab_forg.second = 0;
					for(int i = 0; i < (int)back_vec.size(); i++)
						for(int j = 0; j < (int)back_vec[0].size(); j++){
							if(back_vec[i][j] < ab_back.first && back_vec[i][j] > -1)
								ab_back.first = back_vec[i][j];
							if(back_vec[i][j] > ab_back.second)
								ab_back.second = back_vec[i][j];

							if(forg_vec[i][j] < ab_forg.first && forg_vec[i][j] > -1)
								ab_forg.first = forg_vec[i][j];
							if(forg_vec[i][j] > ab_forg.second)
								ab_forg.second = forg_vec[i][j];
						}
					// cout << "back min, max = " << ab_back.first << ", " << ab_back.second << endl;
					// cout << "forg min, max = " << ab_forg.first << ", " << ab_forg.second << endl;

					// Saves background/foreground
					// DEBUG SAVE BACKGROUND AND FORGROUND IMAGES
					back_img.save();
					forg_img.save();
					back_img = hist_fname_back_hs, forg_img = hist_fname_forg_hs;
					
					// Create before histograms
					utilities::histCreate(back_vec, hist_before_back, pair<int,int>{0, 0}, size);
					utilities::histCreate(forg_vec, hist_before_forg, pair<int,int>{0, 0}, size);

					// Stretch histograms
					utilities::histStretchGS(back_vec, pair<int,int>{0, 0}, size, ab_back);
					utilities::histStretchGS(forg_vec, pair<int,int>{0, 0}, size, ab_forg);

					// Create new histograms after stretch
					utilities::histCreate(back_vec, hist_after_back, pair<int,int>{0, 0}, size);
					utilities::histCreate(forg_vec, hist_after_forg, pair<int,int>{0, 0}, size);

					// Adds stretched vector into images background/foreground 
					utilities::addVectorToImage(back_img, back_vec, start, size);
					utilities::addVectorToImage(forg_img, forg_vec, start, size);

					// DEBUG SAVE BACKGROUND AND FORGROUND IMAGES
					back_img.save();
					forg_img.save();
					
					// Saves historgrams before and after
					utilities::histSave(hist_before_back, hist_after_back, hist_fname_back);
					utilities::histSave(hist_before_forg, hist_after_forg, hist_fname_back);


					// Combines back/fore ground with tgt image
					utilities::addVectorToImage(tgt, back_vec, start, size);
					utilities::addVectorToImage(tgt, forg_vec, start, size);
				}
			}

			// Histogram background/foreground stretching bilinear
			else if(!strncasecmp(pch.c_str(),"histgsbloth",MAXLEN)) {
				// Stuff
				pair<int, int> ab, cd;
				int hist_before_back[256] = {0}, hist_after_back[256] = {0};
				int hist_before_forg[256] = {0}, hist_after_forg[256] = {0};
				int thresh_opt;
				string stemp(to_string(count));
				string hist_fname_back = outfile + "-Histogram-" + stemp + "_background.pgm";
				string hist_fname_forg = outfile + "-Histogram-" + stemp + "_forground.pgm";
				string hist_fname_back_hs = outfile + "-hs-" + stemp + "_background.pgm";
				string hist_fname_forg_hs = outfile + "-hs-" + stemp + "_forground.pgm";
				string hist_fname_back_nhs = outfile + "-background-" + stemp + ".pgm";
				string hist_fname_forg_nhs = outfile + "-forground-" + stemp + ".pgm";
				image back_img, forg_img;
				
				back_img = src, forg_img = src;
				back_img = hist_fname_back_nhs, forg_img = hist_fname_forg_nhs;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				fp >> ab.first >> ab.second;
				fp >> cd.first >> cd.second;

				// If no overlap
				if(!ovlap){
					// Get optimal threshold and separate background/foreground
					thresh_opt = utilities::optimalThreshGS(src, start, size);
					vector<vector<int> > back_vec = utilities::threshGSbr(src, back_img, start, size, thresh_opt);
					vector<vector<int> > forg_vec = utilities::threshGSfg(src, forg_img, start, size, thresh_opt);

					// Saves background/foreground
					// DEBUG SAVE BACKGROUND AND FORGROUND IMAGES
					back_img.save();
					forg_img.save();
					back_img = hist_fname_back_hs, forg_img = hist_fname_forg_hs;
					
					// Create before histograms
					utilities::histCreate(back_vec, hist_before_back, pair<int,int>{0, 0}, size);
					utilities::histCreate(forg_vec, hist_before_forg, pair<int,int>{0, 0}, size);

					// Stretch histograms
					utilities::histStretchGS(back_vec, pair<int,int>{0, 0}, size, ab, cd);
					utilities::histStretchGS(forg_vec, pair<int,int>{0, 0}, size, ab, cd);

					// Create new histograms after stretch
					utilities::histCreate(back_vec, hist_after_back, pair<int,int>{0, 0}, size);
					utilities::histCreate(forg_vec, hist_after_forg, pair<int,int>{0, 0}, size);

					// Adds stretched vector into images background/foreground 
					utilities::addVectorToImage(back_img, back_vec, start, size);
					utilities::addVectorToImage(forg_img, forg_vec, start, size);

					// DEBUG SAVE BACKGROUND AND FORGROUND IMAGES
					back_img.save();
					forg_img.save();
					
					// Saves historgrams before and after
					utilities::histSave(hist_before_back, hist_after_back, hist_fname_back);
					utilities::histSave(hist_before_forg, hist_after_forg, hist_fname_back);


					// Combines back/fore ground with tgt image
					utilities::addVectorToImage(tgt, back_vec, start, size);
					utilities::addVectorToImage(tgt, forg_vec, start, size);
				}
			}

			// Histogram Stretching HSI on I
			else if(!strncasecmp(pch.c_str(),"histhsii",MAXLEN)) {
				// Stuff
				pair<int, int> ab;
				int hist_before[256] = {0}, hist_after[256] = {0};
				string stemp(to_string(count));
				string hist_fname = outfile + "-Histogram-" + stemp + ".pgm";
				hsi src_hsi = tgt;
				vector<vector<double> > vecI;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				// fp >> ab.first >> ab.second;

				// If no overlap
				if(!ovlap){
					// Gets min and max
					int int_val;
					ab.first = 1000, ab.second = 0;
					vecI = src_hsi.getIvec();
					for(int i = start.second; i < start.second + size.second; i++)
						for(int j = start.first; j < start.first + size.first; j++){
							int_val = vecI[i][j];

							if(int_val < ab.first)
								ab.first = int_val;
							if(int_val > ab.second)
								ab.second = int_val;
						}
					// cout << "min, max = " << ab.first << ", " << ab.second << endl;


					src_hsi.stretchI(ab, pair<int,int>{0, 255}, start, size);

					tgt = src_hsi;

					utilities::histCreate(src, hist_before, start, size);

					// utilities::histCreate(tgt, hist_after, start, size);
					src_hsi.histI(hist_after);

					utilities::histSave(hist_before, hist_after, hist_fname);

				}
			}

			// Histogram Stretching HSI on H and I
			else if(!strncasecmp(pch.c_str(),"histhsihi",MAXLEN)) {
				// Stuff
				pair<int, int> ab, ab2;
				int hist_before[256] = {0}, hist_after[256] = {0};
				string stemp(to_string(count));
				string hist_fname = outfile + "-Histogram-" + stemp + ".pgm";
				hsi src_hsi = tgt;
				vector<vector<double> > vecI, vecH;

				// cout << "DEBUG 1\n";

				// Gets difference limit
				fp >> ab2.first >> ab2.second;
				// fp >> ab.first >> ab.second;

				// If no overlap
				if(!ovlap){
					// Gets min and max
					int int_val;
					ab.first = 1000, ab.second = 0;
					// ab2.first = 1000, ab2.second = 0;
					// int int_val2;
					vecI = src_hsi.getIvec();
					vecH = src_hsi.getHvec();
					for(int i = start.second; i < start.second + size.second; i++)
						for(int j = start.first; j < start.first + size.first; j++){
							int_val = vecI[i][j];
							if(int_val < ab.first)
								ab.first = int_val;
							if(int_val > ab.second)
								ab.second = int_val;

							// int_val2 = vecH[i][j];
							// if(int_val2 < ab2.first)
							// 	ab2.first = int_val2;
							// if(int_val2 > ab2.second)
							// 	ab2.second = int_val2;
						}

					// cout << "I min, max = " << ab.first << ", " << ab.second << endl;
					// cout << "H min, max = " << ab2.first << ", " << ab2.second << endl << endl;

					src_hsi.stretchI(ab, pair<int,int>{0, 255}, start, size);
					src_hsi.stretchH(ab2, pair<int,int>{0, 360}, start, size);
					tgt = src_hsi;
					utilities::histCreate(src, hist_before, start, size);
					// utilities::histCreate(tgt, hist_after, start, size);
					src_hsi.histI(hist_after);
					utilities::histSave(hist_before, hist_after, hist_fname);
				}
			}

			// // Histogram equalization
			// else if(!strncasecmp(pch.c_str(),"histgs",MAXLEN)) {
			// 	// Stuff
			// 	int hist_before[256] = {0}, hist_after[256] = {0};
			// 	string stemp(to_string(count));
			// 	string hist_fname = outfile + "-Histogram-" + stemp + ".pgm";

			// 	// cout << "DEBUG 1\n";

			// 	// Gets difference limit
			// 	fp >> ab.first >> ab.second;

			// 	// If no overlap
			// 	if(!ovlap){

			// 		utilities::histStretchGS(src, tgt, start, size, ab);
			// 		utilities::histCreate(src, hist_before, start, size);
			// 		utilities::histCreate(tgt, hist_after, start, size);
			// 		utilities::histSave(hist_before, hist_after, hist_fname);
			// 	}
			// }

			// No valid function
			else {
				cout << "\nNo Function: " << pch << endl;
				break;
			}
		}

		// Saves target
		if(cv_flag)
			imwrite(outfile, tgt_cv);
		else
			tgt.save();
		getline(fp,strtemp);

	}

	// Closes file and calls it a day
	fp.close();
	return 0;
}
