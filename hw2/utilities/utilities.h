#ifndef UTILITIES_H
#define UTILITIES_H

#define HISTMAX 256

#include "image.h"
#include <sstream>
#include <math.h>
#include <algorithm>

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
		static int optimalThreshGS(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int limit = 5);
		static void histCreate(image& src, int hist[256], pair<int, int> start , pair<int, int> stop);
		static void histPrint(int hist[256]); //debug
		static void histSave(int hist[256], string& fname);
		static void histStretchGS(image& src,image& tgt,pair<int,int> ab,pair<int,int> cd,pair<int,int> start,pair<int,int> stop);
		static void stringToChar(string&,char*);
		static void histSave(int hbefore[256],int hafter[256],string& fname);
		static bool stretchInBounds(image& src, string& infile, pair<int,int> start, pair<int,int> size, pair<int,int> ab, pair<int,int> cd);
		static void histCreateRGB(image& src, int hist[256], pair<int, int> start , pair<int, int> size, const int COLOR);
		static void histStretchRGB(image& src, image& tgt, pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size, const int COLOR);
		static void histStretchRGB(image& src, image& tgt, pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size);
		static void histCreateRGB(image& src, int histr[256], int histg[256], int histb[256], pair<int,int> start, pair<int,int> size);
};

#endif
