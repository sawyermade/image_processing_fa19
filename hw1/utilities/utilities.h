#ifndef UTILITIES_H
#define UTILITIES_H

#define HISTMAX 256

#include "image.h"
#include <sstream>
#include <math.h>

class utilities
{
	public:
		utilities();
		virtual ~utilities();
		static std::string intToString(int number);
		static int checkValue(int value);

		//hw1
		static bool roi_overlap(vector<vector<int> > rois, vector<int> roi);
		static void roiBinarizeRange(image &src, image &tgt, pair<int, int>startPoints, pair<int, int> sizexy, int thresh1, int thresh2);
		static void roiBinarizeColor(image &src, image &tgt, int threshold, int colors[], pair<int, int> startPoints, pair<int, int> sizexy);
		static void roiSmooth2DAdaptive(image& src, image& tgt, int ws, pair<int, int> startPoints, pair<int, int> size);
};

#endif
