#ifndef UTILITIES_H
#define UTILITIES_H

#define HISTMAX 256

#include "image.h"
#include <sstream>
#include <math.h>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class utilities
{
	public:
		utilities();
		virtual ~utilities();
		static std::string intToString(int number);
		static int checkValue(int value);

		// hw1
		static bool roi_overlap(vector<vector<int> > rois, vector<int> roi);
		static void roiBinarizeRange(image &src, image &tgt, pair<int, int>startPoints, pair<int, int> sizexy, int thresh1, int thresh2);
		static void roiBinarizeColor(image &src, image &tgt, int threshold, int colors[], pair<int, int> startPoints, pair<int, int> sizexy);
		static void roiSmooth2DAdaptive(image& src, image& tgt, int ws, pair<int, int> startPoints, pair<int, int> size);
		static void roiSmooth1DAdaptive(image& src, image& tgt, int ws, pair<int, int> start, pair<int, int> size);

		// hw2
		static void addVectorToImage(image& tgt, vector<vector<int> >& pixels, pair<int,int> start, pair<int,int> size);
		static vector<vector<int> > threshGSbr(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh);
		static vector<vector<int> > threshGSfg(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh);
		static void threshGS(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh);
		static int optimalThreshGS(image& src, pair<int, int> start, pair<int, int> size, int limit = 5);
		static void histCreate(image& src, int hist[256], pair<int, int> start , pair<int, int> stop);
		static void histCreate(vector<vector<int> >& src, int hist[256], pair<int, int> start , pair<int, int> stop);
		static void histPrint(int hist[256]); //debug
		static void histSave(int hist[256], string& fname);
		static void histStretchGS(image& src,image& tgt, pair<int,int> start, pair<int,int> stop, pair<int,int> ab, pair<int,int> cd = {0, 255});
		static void histStretchGS(vector<vector<int> >& tgt, pair<int,int> start, pair<int,int> stop, pair<int,int> ab, pair<int,int> cd = {0, 255});
		static void stringToChar(string&,char*);
		static void histSave(int hbefore[256],int hafter[256],string& fname);
		static bool stretchInBounds(image& src, string& infile, pair<int,int> start, pair<int,int> size, pair<int,int> ab, pair<int,int> cd);
		static void histCreateRGB(image& src, int hist[256], pair<int, int> start , pair<int, int> size, const int COLOR);
		static void histStretchRGB(image& src, image& tgt, pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size, const int COLOR);
		static void histStretchRGB(image& src, image& tgt, pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size);
		static void histCreateRGB(image& src, int histr[256], int histg[256], int histb[256], pair<int,int> start, pair<int,int> size);

		// hw3
		static void gradient2d(image& src, image& tgt, image& tgtd, pair<int, int> start, pair<int, int> size, vector<vector<int> > kernelx, vector<vector<int> > kernely);
		static void binarizegs(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh, int ws);
		static void binarizedeg(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh, int ws);
		static void binarizegsdeg(image& src_grad, image& src_deg, image& tgt, pair<int, int> start, pair<int, int> size, int thresh_grad, int thresh_deg, int ws);

		// hw4
		static void swapQuads(cv::Mat &img);
		static void cvdft(cv::Mat &src, cv::Mat &magI, cv::Mat &complexI_out);
		static void cvidft(cv::Mat &tgt, cv::Mat &magI, cv::Mat &complexI);
		static void dftlp(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0);
		static void dfthp(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0);
		static void dftn(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0, int d1);
		static void dftlpn(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0, int d1, int d2);
		static void dfthpn(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0, int d1, int d2);
};

#endif
