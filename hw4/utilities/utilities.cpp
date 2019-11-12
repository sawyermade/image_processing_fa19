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

void utilities::swapQuads(cv::Mat &img) {
	// Rearranges amplitude image
	// img = img(cv::Rect(0, 0, img.cols & -2, img.rows & -2));
	int cx = img.cols/2;
	int cy = img.rows/2;
	cv::Mat q0(img, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    cv::Mat q1(img, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(img, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(img, cv::Rect(cx, cy, cx, cy)); // Bottom-Right
    // swap quadrants (Top-Left with Bottom-Right)
    cv::Mat tmp;                           
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    // swap quadrant (Top-Right with Bottom-Left)
    q1.copyTo(tmp);                    
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

void utilities::cvidft(cv::Mat &tgt, cv::Mat &magI, cv::Mat &complexI){
	// Creates fourier domain amplitude image
	double minv, maxv;
	cv::minMaxLoc(tgt, &minv, &maxv);
	cv::Mat planes[2];
	cv::split(complexI, planes);
	cv::magnitude(planes[0], planes[1], magI);
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);
	cv::normalize(magI, magI, minv, maxv, CV_MINMAX);

	// Converts back saves to target
	cv::Mat temp, channels[2];
	tgt.copyTo(temp);
	utilities::swapQuads(complexI);
	cv::idft(complexI, temp);
	cv::normalize(temp, temp, minv, maxv, CV_MINMAX);
	temp.convertTo(temp, CV_8U);
	cv::split(temp, channels);
	channels[0].copyTo(tgt);
}

void utilities::cvdft(cv::Mat &src, cv::Mat &magI, cv::Mat &complexI){
	// Gets min/max
	double minv, maxv;
	cv::minMaxLoc(src, &minv, &maxv);

	// Pads image if needed
	cv::Mat padded;
	int m = cv::getOptimalDFTSize(src.rows);
	int n = cv::getOptimalDFTSize(src.cols);
	cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	// Create planes and run dft
	cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
	cv::merge(planes, 2, complexI);
	cv::dft(complexI, complexI);

	// Swaps quadrants
	utilities::swapQuads(complexI);

	// Creates fourier domain amplitude image
	cv::split(complexI, planes);
	cv::magnitude(planes[0], planes[1], magI);
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);
	cv::normalize(magI, magI, minv, maxv, CV_MINMAX);
}

void utilities::dftlp(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0){
	// Runs dft on src
	cv::Mat complexI;
	utilities::cvdft(tgt, magbefore, complexI);

	// Runs low pass filter mask
	cv::Mat mask(complexI.rows, complexI.cols, CV_32F);
	int cx = complexI.cols / 2, cy = complexI.rows / 2, di;
	for(int i = 0; i < complexI.rows; i++){
		for(int j = 0; j < complexI.cols; j++){
			di = sqrt(pow(i-cy, 2) + pow(j-cx, 2));
			if(di <= d0)
				mask.at<float>(i, j) = 1;
			else
				mask.at<float>(i, j) = 0;
		}
	}

	// Applies mask to complexI
	cv::Mat planes[2];
	cv::split(complexI, planes);
	cv::multiply(planes[0], mask, planes[0]);
	cv::multiply(planes[1], mask, planes[1]);
	cv::merge(planes, 2, complexI);

	// Inverse DFT
	utilities::cvidft(tgt, magafter, complexI);
}

void utilities::dfthp(cv::Mat &tgt, cv::Mat &magbefore, cv::Mat &magafter, int d0){
	// Runs dft on src
	cv::Mat complexI;
	utilities::cvdft(tgt, magbefore, complexI);

	// Runs low pass filter mask
	cv::Mat mask(complexI.rows, complexI.cols, CV_32F);
	int cx = complexI.cols / 2, cy = complexI.rows / 2, di;
	for(int i = 0; i < complexI.rows; i++){
		for(int j = 0; j < complexI.cols; j++){
			di = sqrt(pow(i-cy, 2) + pow(j-cx, 2));
			if(di >= d0)
				mask.at<float>(i, j) = 1;
			else
				mask.at<float>(i, j) = 0;
		}
	}

	// Applies mask to complexI
	cv::Mat planes[2];
	cv::split(complexI, planes);
	cv::multiply(planes[0], mask, planes[0]);
	cv::multiply(planes[1], mask, planes[1]);
	cv::merge(planes, 2, complexI);

	// Inverse DFT
	utilities::cvidft(tgt, magafter, complexI);
}

void utilities::binarizegsdeg(image& src_grad, image& src_deg, image& tgt, pair<int, int> start, pair<int, int> size, int thresh_grad, int thresh_deg, int ws) {
	// Local vars
	int x1, y1, x2, y2, m, pixel_val_deg, pixel_val_grad, max_deg, min_deg;
	x1 = start.first, x2 = x1 + size.first;
	y1 = start.second, y2 = y1 + size.second;
	m = (ws - 1) / 2;

	// Find degree bounds
	max_deg = thresh_deg + 10;
	min_deg = thresh_deg - 10;
	if(max_deg > 360)
		max_deg -= 360;
	if(min_deg < 0)
		min_deg += 360;

	// Goes through roi
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Checks for boarder, makes black
			if(i < y1+m || i >= y2-m || j < x1+m || j >= x2-m){
				tgt.setAll(i, j, BLACK);
				continue;
			}

			// Thresholds by degree and gradient
			pixel_val_deg = src_deg.getPixel(i, j);
			pixel_val_grad = src_grad.getPixel(i, j);
			if(pixel_val_deg <= max_deg && pixel_val_deg >= min_deg && pixel_val_grad >= thresh_grad)
				tgt.setAll(i, j, WHITE);
			else
				tgt.setAll(i, j, BLACK);
		}
	}
}

void utilities::binarizegs(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh, int ws){
	// Local vars
	int x1, y1, x2, y2, m, pixel_val;
	x1 = start.first, x2 = x1 + size.first;
	y1 = start.second, y2 = y1 + size.second;
	m = (ws - 1) / 2;

	// Goes through roi
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Checks for boarder, makes black
			if(i < y1+m || i >= y2-m || j < x1+m || j >= x2-m){
				tgt.setAll(i, j, BLACK);
				continue;
			}

			pixel_val = src.getPixel(i, j);
			if(pixel_val < thresh)
				tgt.setAll(i, j, BLACK);
			else
				tgt.setAll(i, j, WHITE);
		}
	}
}

void utilities::binarizedeg(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh, int ws){
	// Local vars
	int x1, y1, x2, y2, m, pixel_val, max_deg, min_deg;
	x1 = start.first, x2 = x1 + size.first;
	y1 = start.second, y2 = y1 + size.second;
	m = (ws - 1) / 2;

	// Find degree bounds
	max_deg = thresh + 10;
	min_deg = thresh - 10;
	if(max_deg > 360)
		max_deg -= 360;
	if(min_deg < 0)
		min_deg += 360;

	// Goes through roi
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Checks for boarder, makes black
			if(i < y1+m || i >= y2-m || j < x1+m || j >= x2-m){
				tgt.setAll(i, j, BLACK);
				continue;
			}

			pixel_val = src.getPixel(i, j);
			if(pixel_val <= max_deg && pixel_val >= min_deg)
				tgt.setAll(i, j, WHITE);
			else
				tgt.setAll(i, j, BLACK);
		}
	}
}

void utilities::gradient2d(image& src, image& tgt, image& tgtd, pair<int, int> start, pair<int, int> size, vector<vector<int> > kernelx, vector<vector<int> > kernely) {
	// Local vars
	int pixel_val, pixel_new, x1, y1, x2, y2, m, ws, sumx, sumy;
	double max_grad, max_mag, magnitude, direction;
	x1 = start.first, x2 = x1 + size.first;
	y1 = start.second, y2 = y1 + size.second;
	ws = kernelx.size(), m = (ws - 1) / 2;

	// Calc max gradient value for normalization
	max_grad = 0;
	for(int i = 0; i < (int)kernelx.size(); i++){
		for(int j = 0; j < (int)kernelx[0].size(); j++){
			pixel_val = kernelx[i][j];
			if(pixel_val > 0)
				max_grad += pixel_val;
		}
	}
	max_grad *= MAXRGB * 3;
	max_mag = sqrt(2 * max_grad * max_grad);
	if((int)max_mag < max_mag)
		max_mag = (int)max_mag + 1;

	// Goes through all the pixels in the ROI and convolves them with kernel
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Checks for boarder, makes black
			if(i < y1+m || i >= y2-m || j < x1+m || j >= x2-m){
				tgt.setAll(i, j, BLACK);
				continue;
			}

			// Goes through window and kernel
			sumx = 0, sumy = 0;		
			for(int k = i-m; k < i+m+1; k++){
				for(int l = j-m; l < j+m+1; l++){
					// Get pixel value grayscale and color
					pixel_val = 
						src.getPixel(k, l, RED) + 
						src.getPixel(k, l, GREEN) + 
						src.getPixel(k, l, BLUE);

					// Get weighted sum
					sumx += kernelx[k-i+m][l-j+m]*pixel_val;
					sumy += kernely[k-i+m][l-j+m]*pixel_val;
				}
			}

			// Calcs magnitude and creates edge image in tgt
			magnitude = sqrt((sumx*sumx) + (sumy*sumy));
			pixel_new = (int)(magnitude / max_mag * 255);
			tgt.setAll(i, j, checkValue(pixel_new));
			// cout << "magnitude = " << pixel_new << ", old = " << src.getPixel(i, j) << endl;

			// Calc direction with range 0 to 360 and makes direction image tgtd
			direction = atan2(sumy, sumx) * 180 / M_PI;
			// cout << "direction = " << direction << endl;
			if(direction < 0)
				direction += 360;
			tgtd.setAll(i, j, (int)direction);
			// cout << "direction = " << direction << endl << endl;
		}
	}
}

void utilities::addVectorToImage(image& tgt, vector<vector<int> >& pixels, pair<int,int> start, pair<int,int> size) {
	// Local vars
	int pixel_val, x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;

	// Goes through ROI and copies pixels
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Gets pixel val
			pixel_val = pixels[i-y1][j-x1];

			// Checks if greater than -1
			if(pixel_val > -1) {
				tgt.setPixel(i, j, pixel_val);
			}

		}
	}
}

vector<vector<int> > utilities::threshGSbr(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh){
	// Goes through ROI
	uint8_t pixel_val;
	int x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;
	vector<vector<int> > pixels(y2-y1, vector<int>(x2-x1, -1));
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Gets pixel val
			pixel_val = src.getPixel(i, j);

			// Thresholds, less black
			if(pixel_val >= thresh){
				tgt.setPixel(i, j, 0);
			}
			else{
				pixels[i-y1][j-x1] = pixel_val;
			}
		}
	}

	return pixels;
}

vector<vector<int> > utilities::threshGSfg(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh){
	// Goes through ROI

	uint8_t pixel_val;
	int x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;
	vector<vector<int> > pixels(y2-y1, vector<int>(x2-x1, -1));
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Gets pixel val
			pixel_val = src.getPixel(i, j);

			// Thresholds, less black
			if(pixel_val < thresh)
				tgt.setPixel(i, j, 0);
			else
				pixels[i-y1][j-x1] = pixel_val;
		}
	}

	return pixels;
}

void utilities::threshGS(image& src, image& tgt, pair<int, int> start, pair<int, int> size, int thresh){
	// Goes through ROI
	uint8_t pixel_val;
	int x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;
	for(int i = y1; i < y2; i++){
		for(int j = x1; j < x2; j++){
			// Gets pixel val
			pixel_val = src.getPixel(i, j);

			// Thresholds, less black
			if(pixel_val < thresh)
				tgt.setPixel(i, j, 0);
			else
				tgt.setPixel(i, j, 255);
		}
	}
}

int utilities::optimalThreshGS(image& src, pair<int, int> start, pair<int, int> size, int limit){
	// Local vars

	vector<uint8_t> median_vec;
	int median_val, thresh_prev, thresh_curr, len_vec;
	int x1 = start.first, y1 = start.second, x2 = x1 + size.first, y2 = y1 + size.second;
	int pixel_val, back_mean, back_count, forg_mean, forg_count, diff;

	// Copies pixels to median vector
	for(int i = y1; i < y2; i++)
		for(int j = x1; j < x2; j++)
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
		for(int i = y1; i < y2; i++){
			for(int j = x1; j < x2; j++){
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

void utilities::stringToChar(string& str, char* ch) {

	for(int i = 0; i < (int)str.size(); ++i)
		ch[i] = str[i];
	ch[str.size()] = '\0';
}

void utilities::histStretchGS(image& src, image& tgt, pair<int,int> start, pair<int,int> stop, pair<int,int> ab, pair<int,int> cd) {

	double ratio, tempd;
	int temp;

	stop.first += start.first;
	stop.second += start.second;

	ratio = ((double)cd.second - (double)cd.first) / ((double)ab.second - (double)ab.first);

	//converts pixels using Inew = (d-c)/(b-a)[Iij - a] + c
	for(int i = start.second; i < stop.second; ++i) {
		for(int j = start.first; j < stop.first; ++j) {
			
			if(src.getPixel(i,j) < ab.first)
				tgt.setPixel(i,j,cd.first);

			else if(src.getPixel(i,j) > ab.second)
				tgt.setPixel(i,j,cd.second);

			else {
				tempd = ratio * (double)(src.getPixel(i,j) - ab.first) + cd.first;
				temp = tempd;
				tgt.setPixel(i,j,checkValue(temp));
			}
		}
	}
}

void utilities::histStretchGS(vector<vector<int> >& tgt, pair<int,int> start, pair<int,int> stop, pair<int,int> ab, pair<int,int> cd) {

	double ratio, tempd;
	int temp, pixel_val;

	stop.first += start.first;
	stop.second += start.second;

	ratio = ((double)cd.second - (double)cd.first) / ((double)ab.second - (double)ab.first);

	// cout << "a, b = " << ab.first << ", " << ab.second << endl;

	//converts pixels using Inew = (d-c)/(b-a)[Iij - a] + c
	for(int i = start.second; i < stop.second; ++i) {
		for(int j = start.first; j < stop.first; ++j) {
			// Gets pixel val
			pixel_val = tgt[i][j];
			if(pixel_val < 0)
				continue;
			
			if(pixel_val < ab.first)
				tgt[i][j] = cd.first;

			else if(pixel_val > ab.second)
				tgt[i][j] = cd.second;

			else {
				tempd = ratio * (double)(tgt[i][j] - ab.first) + cd.first;
				temp = tempd;
				tgt[i][j] = temp;
			}
		}
	}
}


//
void utilities::histSave(int hist[256], string& fname) {

	//cout << "\nIn HistSave" << endl;

	//normalize hist array;
	int max = 0, min;

	for(int i = 0; i < 256; ++i)
		if(max < hist[i])
			max = hist[i];

	min = max;
	for(int i = 0; i < 256; ++i)
		if(min > hist[i])
			min = hist[i];

	//double ratio = 256.0 / (double)max;


	//tgt.resize(HISTMAX,HISTMAX);
	double ratio;
	image tgt(HISTMAX,HISTMAX,fname);

	//
	for(int j = 0; j < HISTMAX; ++j) {

		ratio = (double)(hist[j] - min) / (double)(max - min);
		//cout << ratio << "  ";
		for(int i = 0; i < HISTMAX; ++i) {

			if(i < 255 - (255 *ratio))
				tgt.setAll(i,j,BLACK);
			else
				tgt.setAll(i,j,WHITE);
		}
	}

	tgt.save();
}

void utilities::histSave(int hbefore[256],int hafter[256],string& fname) {

	//normalize hist array;
	int maxbef = 0, maxaft = 0, minbef, minaft;

	for(int i = 0; i < 256; ++i) {
		if(maxbef < hbefore[i])
			maxbef = hbefore[i];
		if(maxaft < hafter[i])
			maxaft = hafter[i];
	}

	minbef = maxbef, minaft = maxaft;
	for(int i = 0; i < 256; ++i) {
		if(minbef > hbefore[i])
			minbef = hbefore[i];
		if(minaft > hafter[i])
			minaft = hafter[i];
	}

	//variables for the two histograms before and after.
	int size = 512;
	double ratio, ratioa;
	image tgt(size,size,fname), temp(size,size);

	
	for(int j = 0; j < HISTMAX; ++j) {

		ratio = (double)(hbefore[j] - minbef) / (double)(maxbef - minbef);
		ratioa = (double)(hafter[j] - minaft) / (double)(maxaft - minaft);
		//cout << ratio << "  ";
		for(int i = 0; i < size; ++i) {

			if(i < (size-1) - ((size-1) *ratio)) {
				//tgt.setPixel(i,j,BLACK); //for size = 256

				tgt.setAll(i,j*2,BLACK);
				tgt.setAll(i,j*2+1,BLACK);
			}
			else {
				//tgt.setPixel(i,j,WHITE); //for size = 256

				tgt.setAll(i,j*2,WHITE);
				tgt.setAll(i,j*2+1,WHITE);
			}

			if(i < (size-1) - ((size-1) *ratioa)) {
				//temp.setPixel(i,j,BLACK); //for size = 256

				temp.setAll(i,j*2,BLACK);
				temp.setAll(i,j*2+1,BLACK);
			}
			else {
				//temp.setPixel(i,j,WHITE); //for size = 256

				temp.setAll(i,j*2,WHITE);
				temp.setAll(i,j*2+1,WHITE);
			}
		}
	}

	tgt += temp;
	tgt.save();
}

void utilities::histCreate(image& src, int hist[256], pair<int, int> start, pair<int, int> stop) {
	
	// stop.first += start.first, stop.second += start.second;

	for(int i = start.second; i < stop.second + start.second; ++i)
		for(int j = start.first; j < stop.first + start.first; ++j)
			++hist[src.getPixel(i,j)];
}

void utilities::histCreate(vector<vector<int> >& src, int hist[256], pair<int, int> start, pair<int, int> stop) {
	
	stop.first += start.first, stop.second += start.second;

	for(int i = start.second; i < stop.second; ++i)
		for(int j = start.first; j < stop.first; ++j){
			if(src[i][j] > -1)
				++hist[src[i][j]];
		}
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
	for(int i = start.second; i < end.second; i++) {

		for(int j = start.first; j < end.first; j++) {
			//sets m value
			for(m = m_max; m > 0; m--){
				// Checks top left areas
				if(j - start.first < m || i - start.second < m)
					continue;

				// Checks bottom right areas
				if(end.first - j < m || end.second - i < m)
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
			for(int x = i-m; x < i+m+1; x++){
				for(int y = j-m; y < j+m+1; y++){
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