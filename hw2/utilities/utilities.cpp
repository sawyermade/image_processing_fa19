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

int utilities::optimalThreshGS(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int limit){
	// Local vars
	vector<uint8_t> median_vec;
	int median_val, thresh_prev, thresh_curr, len_vec;
	int x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;
	int pixel_val, back_mean, back_count, forg_mean, forg_count, diff;

	// Copies pixels to median vector
	for(int j = y1; j < y2; j++)
		for(int i = x1; i < x2; i++)
			median_vec.push_back(src.getPixel(i, j));

	// Sorts and gets median value
	std::sort(median_vec.begin(), median_vec.end());
	len_vec = median_vec.size();
	if(len_vec % 2 == 0){
		int m2 = len_vec / 2, m1 = m2 - 1;
		median_val = (median_vec[m1] + median_vec[m2]) / 2;
	}
	else
		median_val = median_vec[len_vec / 2];

	// Iterates until difference below limit
	thresh_curr = median_val;
	do {
		// Goes through every src pixel and sets background/foreground
		back_mean = 0, forg_mean = 0;
		back_count = 0; forg_count = 0;
		for(int j = y1; j < y2; j++){
			for(int i = x1; i < x2; i++){
				// Gets pixel value
				pixel_val = src.getPixel(i, j);

				// Checks if forground or background
				if(pixel_val <= thresh_curr){
					back_mean += pixel_val;
					back_count++;
				}
				else{
					forg_mean += pixel_val;
					forg_count++;
				}
			}
		}

		// Cals means and difference
		back_mean /= back_count;
		forg_mean /= forg_count;
		thresh_prev = thresh_curr;
		thresh_curr = (back_mean + forg_mean) / 2;
		diff = std::abs(thresh_prev - thresh_curr);
	}
	while(diff >= limit);

	// Returns optimal thresholding value
	return thresh_curr;
}

void utilities::histCreate(image& src, int hist[256], pair<int, int> start, pair<int, int> stop) {
	
	stop.first += start.first, stop.second += start.second;

	for(int i = start.first; i < stop.first; ++i)
		for(int j = start.second; j < stop.second; ++j)
			++hist[src.getPixel(i,j)];
}

//debug
void utilities::histPrint(int hist[256]) {

	int sum = 0;

	for(int i = 0; i < 256; ++i) {
		cout << hist[i] << endl;
		sum += hist[i];
	}

	cout << "\nSUM = " << sum << endl;
}

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

		// If there is overlap, return true
		if(x_tl_max < x_br_min && y_tl_max < y_br_min)
			return true;
	}

	// No overlap, return false
	return false;
}

// Binarize/Thresholding for grayscale. Runs at O(n^2)
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
void utilities::roiBinarizeColor(image &src, image &tgt, int threshold, int colors[], pair<int, int> startPoints, pair<int, int> sizexy) {
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

			//calculates the windows' pixel sum for rgb/3 channel gray scale.
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

//1D Adaptive Window Smooth. Runs at O(2n^2 + 2m) where m << n, so runs at O(n^2).
void utilities::roiSmooth1DAdaptive(image& src, image& tgt, int ws, pair<int, int> start, pair<int, int> size) {
	// Vars
	int sumr, sumg, sumb, k, m_max = (ws-1)/2, m;
	image intermediate;
	pair<int, int> end;

	//copies image and calculates endpoints.
	intermediate = src;
	end.first = start.first + size.first;
	end.second = start.second + size.second;

	//Horizontal 1D
	for(int j = start.second; j < end.second; ++j) {
		// Sums for averages
		sumr = 0, sumg = 0, sumb = 0;
		k = start.second;

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
			ws = 2*m + 1;

			if(j <= start.second + m) {
		
				for(; k <= j+m; ++k) {
					sumr += src.getPixel(i,k);
					sumg += src.getPixel(i,k,GREEN);
					sumb += src.getPixel(i,k,BLUE);
				}

				intermediate.setPixel(i,j,checkValue(sumr/(k - start.second)));
				intermediate.setPixel(i,j,GREEN,checkValue(sumg/(k - start.second)));
				intermediate.setPixel(i,j,BLUE,checkValue(sumb/(k - start.second)));
			}

			else if(j >= end.second - m) {

				sumr -= src.getPixel(i,j-1-m);
				sumg -= src.getPixel(i,j-1-m,GREEN);
				sumb -= src.getPixel(i,j-1-m,BLUE);

				intermediate.setPixel(i,j,checkValue(sumr/(end.second - j + m)));
				intermediate.setPixel(i,j,GREEN,checkValue(sumg/(end.second - j + m)));
				intermediate.setPixel(i,j,BLUE,checkValue(sumb/(end.second - j + m)));
			}

			else {

				sumr = sumr - src.getPixel(i,j-1-m) + src.getPixel(i,j+m);
				sumg = sumg - src.getPixel(i,j-1-m,GREEN) + src.getPixel(i,j+m,GREEN);
				sumb = sumb - src.getPixel(i,j-1-m,BLUE) + src.getPixel(i,j+m,BLUE);

				intermediate.setPixel(i,j,checkValue(sumr/ws));
				intermediate.setPixel(i,j,GREEN,checkValue(sumg/ws));
				intermediate.setPixel(i,j,BLUE,checkValue(sumb/ws));
			}

		}
	}

	//Vertical 1D
	for(int j = start.second; j < end.second; ++j) {
		// Sums for averages
		sumr = 0, sumg = 0, sumb = 0;
		k = start.first;
		
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
			ws = 2*m + 1;
			
			if(i <= start.first + m) {

				for(; k <= i+m; ++k) {
					sumr += intermediate.getPixel(k,j);
					sumg += intermediate.getPixel(k,j,GREEN);
					sumb += intermediate.getPixel(k,j,BLUE);
				}
				tgt.setPixel(i,j,checkValue(sumr/(k - start.first)));
				tgt.setPixel(i,j,GREEN,checkValue(sumg/(k - start.first)));
				tgt.setPixel(i,j,BLUE,checkValue(sumb/(k - start.first)));
			}

			else if(i >= end.first - m) {

				sumr -= intermediate.getPixel(i-1-m,j);
				sumg -= intermediate.getPixel(i-1-m,j,GREEN);
				sumb -= intermediate.getPixel(i-1-m,j,BLUE);

				tgt.setPixel(i,j,checkValue(sumr/(end.first - i + m)));
				tgt.setPixel(i,j,GREEN,checkValue(sumg/(end.first - i + m)));
				tgt.setPixel(i,j,BLUE,checkValue(sumb/(end.first - i + m)));
			}

			else {

				sumr = sumr - intermediate.getPixel(i-1-m, j) + intermediate.getPixel(i+m,j);
				sumg = sumg - intermediate.getPixel(i-1-m, j,GREEN) + intermediate.getPixel(i+m,j,GREEN);
				sumb = sumb - intermediate.getPixel(i-1-m, j,BLUE) + intermediate.getPixel(i+m,j,BLUE);

				tgt.setPixel(i,j,checkValue(sumr/ws));
				tgt.setPixel(i,j,GREEN,checkValue(sumg/ws));
				tgt.setPixel(i,j,BLUE,checkValue(sumb/ws));
			}
		}
	}
}