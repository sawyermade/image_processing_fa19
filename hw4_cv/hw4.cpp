#include<iostream>
#include<fstream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

char W = ' ';

enum processes {hist_stretch, hist_equal, sobel, canny, sobel_hist, canny_hist};
processes hash_proc(string const &process){
	if(process == "hist_stretch") return hist_stretch;
	if(process == "hist_equal") return hist_equal;
	if(process == "sobel") return sobel;
	if(process == "canny") return canny;
	if(process == "sobel_hist") return sobel_hist;
	if(process == "canny_hist") return canny_hist;
}

void hist_cv(Mat &img_roi, int channel_num){
	if(channel_num > 1){
		Mat channels[3];
		cvtColor(img_roi, img_roi, CV_BGR2HSV);
		split(img_roi, channels);
		equalizeHist(channels[2], channels[2]);
		merge(channels, 3, img_roi);
		cvtColor(img_roi, img_roi, CV_HSV2BGR);
	}
	else{
		equalizeHist(img_roi, img_roi);
	}
}

void sobel_cv(Mat &img_roi, int channel_num){
	Mat gx, gy;
	int ksize = 3, ddepth = -1;
	if(channel_num > 1)
		cvtColor(img_roi, img_roi, CV_BGR2GRAY);
	Sobel(img_roi, gx, ddepth, 1, 0, ksize);
	Sobel(img_roi, gy, ddepth, 0, 1, ksize);
	convertScaleAbs(gx, gx);
	convertScaleAbs(gy, gy);
	addWeighted(gx, 0.5, gy, 0.5, 0, img_roi);
	threshold(img_roi, img_roi, 10, 255, THRESH_BINARY);
	if(channel_num > 1)
		cvtColor(img_roi, img_roi, CV_GRAY2BGR);
}

void canny_cv(Mat &img_roi, int channel_num){
	int ksize = 3, thresh = 50, ratio = 3;
	if(channel_num > 1)
		cvtColor(img_roi, img_roi, CV_BGR2GRAY);
	Canny(img_roi, img_roi, thresh, thresh*ratio, ksize);
	if(channel_num > 1)
		cvtColor(img_roi, img_roi, CV_GRAY2BGR);
}

int main(int argc, char** argv){
	// Opens parameter file
	ifstream params_fp;
	params_fp.open(argv[1],fstream::in);
	if(!params_fp.is_open()) {
		cout << "Cant open parameter file: " << argv[1] << endl;
		return -1;
	}

	// Goes line by line
	string line;
	istringstream params;
	Rect roi;
	Mat img, img_roi;
	while(params_fp){
		// Gets line
		getline(params_fp, line);
		if(!params_fp)
			continue;
		params.clear();
		params.str(line);
		if(!params)
			continue;
		cout << line << endl;

		// Gets input and output filenames
		string input_fname, output_fname;
		params >> input_fname >> output_fname;
		// cout << input_fname << W << output_fname << endl;

		// Opens image, sets up target
		img = imread(input_fname, -1);

		// Gets number of ROIs
		int num_roi;
		params >> num_roi;

		// Runs each ROI and process
		for(int i = 0; i < num_roi; i++){
			// Gets process and ROI x,y and w,h
			string process;
			int x, y, w, h;
			params >> process >> x >> y >> w >> h;
			// cout << process << W << x << W << y << W << w << W << h << endl;

			// Sets images ROI
			roi = Rect(x, y, w, h);
			img_roi = img(roi);

			//Checks for process
			switch(hash_proc(process)){
				// Historgram Equalization
				case hist_equal:
					hist_cv(img_roi, img.channels());
					break;

				// Histogram stretching
				case hist_stretch:
					int alpha, beta;
					params >> alpha >> beta;
					normalize(img_roi, img_roi, alpha, beta, CV_MINMAX);
					break;

				// Sobel
				case sobel:
					sobel_cv(img_roi, img.channels());
					break;

				// Canny
				case canny:
					canny_cv(img_roi, img.channels());
					break;

				// Sobel & Hist
				case sobel_hist:
					hist_cv(img_roi, img.channels());
					sobel_cv(img_roi, img.channels());
					break;

				// Canny & Hist
				case canny_hist:
					hist_cv(img_roi, img.channels());
					canny_cv(img_roi, img.channels());
					break;
			}

			// Copies img_roi to img
			img_roi.copyTo(img(roi));
		}

		// Writes output image
		cout << "Writing Image: " << output_fname << "... ";
		imwrite(output_fname, img);
		cout << "Done.\n" << endl;
	}

	return 0;
}