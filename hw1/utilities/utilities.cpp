#include "utilities.h"


#define MAXRGB 255
#define MINRGB 0

#define WHITE 255
#define BLACK 0

std::string utilities::intToString(int number)
{
   std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

int utilities::checkValue(int value)
{
	if (value > MAXRGB)
		return MAXRGB;
	if (value < MINRGB)
		return MINRGB;
	else
		return value;
}

/*-----------------------------------------------------------------------*/
//MY STUFF
/*-----------------------------------------------------------------------*/

// Checks for overlapping ROIs
bool utilities::roi_overlap(vector<vector<int> > rois, vector<int> roi) {
	// Gets roi values
	int r_x = roi[0], r_y = roi[1], r_xs = roi[2], r_ys = roi[3];

	// Vars for rois and maxes
	int x, y, xs, ys;
	int x_tl_max, y_tl_max, x_br_min, y_br_min;

	// Goes through all the rois
	for(int i = 0; i < int(rois.size()); i++){
		// Reads roi to compare
		x = rois[i][0];
		y = rois[i][1];
		xs = rois[i][2];
		ys = rois[i][3];

		// Gets max/mins of top left and bottom right corners of roi
		x_tl_max = max(x, r_x);
		y_tl_max = max(y, r_y);
		x_br_min = min(x+xs, r_x+r_xs);
		y_br_min = min(y+ys, r_y+r_ys);

		// If there is overlap, return false
		if(x_tl_max < x_br_min && y_tl_max < y_br_min)
			return true;
	}

	// No overlap, return true
	return false;
}

//Binarize/Thresholding for grayscale. Runs at O(n^2).
void utilities::roiBinarizeRange(image &src, image &tgt, pair<int, int> startPoints, pair<int, int> sizexy, int thresh1, int thresh2) {

	//pair for end points x and y. first is x, second is y.
	pair<int, int> endPoints;

	//calculates end points using start points x y and size for x y.
	endPoints.first = startPoints.first + sizexy.first;
	endPoints.second = startPoints.second + sizexy.second;

	//runs through all pixels in ROI and if pixel value less than or equal to threshold it tunrs it white, else black.
	for(int i = startPoints.first; i < endPoints.first; ++i) {
		for(int j = startPoints.second; j < endPoints.second; ++j) {

			if(thresh1 <= src.getPixel(i,j) && src.getPixel(i,j) <= thresh2)
				tgt.setPixel(i,j,WHITE);
			else
				tgt.setPixel(i,j,BLACK);
		}
	}
}


//Binarize/Thresholding for Coloor images. Runs at O(n^2).
void utilities::roiBinarizeColor(image &src, image &tgt, int threshold, int colors[], pair<int, int> startPoints, pair<int, int> sizexy)
{
	//variables for color distance and end points for ROI.
	int dist;
	pair<int, int> endPoints;

	//calculates end points using start points and sizes.
	endPoints.first = startPoints.first + sizexy.first;
	endPoints.second = startPoints.second + sizexy.second;

	//runs through all the pixels in the ROI, calculates color distance and
	//sets pixel distances that are less than or equal to ThresholdColor(TC) to white, the rest to black.
	for (int i = startPoints.first; i < endPoints.first; i++) {
		for (int j = startPoints.second; j < endPoints.second; j++) {

			dist = sqrt(pow(src.getPixel(i,j,0) - colors[0],2) + pow(src.getPixel(i,j,1) - colors[1],2) + pow(src.getPixel(i,j,2) - colors[2],2));

			if(dist <= threshold) {
				tgt.setPixel(i,j,0,WHITE);
				tgt.setPixel(i,j,1,WHITE);
				tgt.setPixel(i,j,2,WHITE);
			}

			else {
				tgt.setPixel(i,j,0,BLACK);
				tgt.setPixel(i,j,1,BLACK);
				tgt.setPixel(i,j,2,BLACK);
			}
		}
	}
}

//2D Adaptive Window Smooth. Runs at O(n^2 * 2m^2) = O(n^2 * m^2).
void utilities::roiSmooth2DAdaptive(image& src, image& tgt, int ws, pair<int, int> start, pair<int, int> size) {
	// Local vars needed
	pair<int, int> end;
	int m_max = (ws-1)/2, m;
	int sumr, sumg, sumb;

	// Calcs end points
	end.first = start.first + size.first;
	end.second = start.second + size.second;

	// Goes through all pixels in ROI
	for(int j = start.second; j < end.second; ++j) {
		for(int i = start.first; i < end.first; ++i) {
			//sets m value
			for(m = m_max; m > 0; m--){
				// Checks top left areas
				if(i - start.first < m || j - start.second < m)
					continue;

				// Checks bottom right areas
				if(end.first - i < m || end.second - j < m)
					continue;

				// Found m that works, breaks loop
				break;
			}

			// Checks if m smaller than 1 which means cant run on this pixel
			if(m < 1)
				continue;

			//calculates the windows pixel average for rgb.
			sumr = 0, sumg = 0, sumb = 0;
			ws = 2*m + 1;
			for(int y = j-m; y < j+m+1; ++y){
				for(int x = i-m; x < i+m+1; ++x){
					sumr += src.getPixel(x,y,RED);
					sumg += src.getPixel(x,y,GREEN);
					sumb += src.getPixel(x,y,BLUE);
				}
			}

			//sets target image's new pixel values for i,j.
			tgt.setPixel(i,j,RED,checkValue(sumr/(ws*ws)));
			tgt.setPixel(i,j,GREEN,checkValue(sumg/(ws*ws)));
			tgt.setPixel(i,j,BLUE,checkValue(sumb/(ws*ws)));
		}
	}
}
