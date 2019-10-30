#include "hsi.h"
#include "image.h"

hsi::~hsi(void) {
	this->fname.clear();
	this->clearVectors();
}

hsi::hsi(void) {
}

hsi::hsi(image& src) {

	this->convertRGB(src);
	this->fname = src.getFname();
}

hsi::hsi(int row, int col, string& fnamestr) {

	this->rows = row;
	this->cols = col;
	this->fname = fnamestr;
}

hsi::hsi(int row, int col) {

	this->rows = row;
	this->cols = col;
}


//OPERATORS
hsi& hsi::operator=(image& right) {
	this->convertRGB(right);
	this->fname = right.getFname();
	return *this;
}

hsi& hsi::operator=(string& right) {
	this->fname = right;
	return *this;
}


//MEMBER FUNCTIONS
void hsi::convertRGB(image& src) {

	this->resize(src);
	this->convertI(src);
	this->convertH(src);
	this->convertS(src);
	
}

void hsi::resize(image& src) {

	this->rows = src.getNumberOfRows();
	this->cols = src.getNumberOfColumns();
	this->clearVectors();

	this->hsiH.resize(rows, vector<double>(cols, 0));
	this->hsiS.resize(rows, vector<double>(cols, 0));
	this->hsiI.resize(rows, vector<double>(cols, 0));

}

void hsi::clearVectors() {
	this->hsiH.clear();
	this->hsiS.clear();
	this->hsiI.clear();
}

void hsi::histI(int hist[256]) {
	
	for(int i = 0; i < 256; ++i)
		hist[i] = 0;
	
	for(int i = 0; i < rows; ++i)
		for(int j = 0; j < cols; ++j)
			++hist[(int)hsiI[i][j]];
}

void hsi::histS(int hist[256]) {
	
	for(int i = 0; i < 256; ++i)
		hist[i] = 0;
	
	for(int i = 0; i < rows; ++i)
		for(int j = 0; j < cols; ++j)
			++hist[(int)(hsiS[i][j]*255.0)];
}

void hsi::histH(int hist[256]) {
	
	for(int i = 0; i < 256; ++i)
		hist[i] = 0;
	
	for(int i = 0; i < rows; ++i)
		for(int j = 0; j < cols; ++j)
			++hist[(int)(hsiH[i][j]/360.0*255.0)];
}

//HSI conversion.
void hsi::convertH(image& src) {

	int r, g, b;
	double calc, theta, degrees = 180.0 / M_PI;

	for(int i = 0; i < src.getNumberOfRows(); ++i) {

		for(int j = 0; j < src.getNumberOfColumns(); ++j) {

			r = src.getPixel(i,j,RED);
			g = src.getPixel(i,j,GREEN);
			b = src.getPixel(i,j,BLUE);

			if(r == g && r == b)
				hsiH[i][j] = 0;

			else {
				
				theta = acos((0.5 * (double)((r-g)+(r-b))) / (double)sqrt(pow(r-g,2) + (r-b)*(g-b) )) * degrees;

				if(b <= g)
					calc = theta;
				else
					calc = 360.0 - theta;

				hsiH[i][j] = calc;
			}
		}
	}
}

//
void hsi::convertS(image& src) {

	int r, b, g;
	double calc;

	for(int i = 0; i < src.getNumberOfRows(); ++i) {

		for(int j = 0; j < src.getNumberOfColumns(); ++j) {

			r = src.getPixel(i,j,RED);
			g = src.getPixel(i,j,GREEN);
			b = src.getPixel(i,j,BLUE);

			if(r == g && r == b)
				hsiS[i][j] = 0.0;
			
			else {
				
				if((r+g+b)/3 == 0)
					calc = 0;
				
				else
					calc = 1.0 - (3.0/(double)(r+g+b) * min(r,min(g,b)));
				
				hsiS[i][j] = calc;
			}
		}
	}	
}

//
void hsi::convertI(image& src) {

	int r, g, b;
	double avg;

	for(int i = 0; i < src.getNumberOfRows(); ++i) {

		for(int j = 0; j < src.getNumberOfColumns(); ++j) {

			r = src.getPixel(i,j,RED);
			g = src.getPixel(i,j,GREEN);
			b = src.getPixel(i,j,BLUE);

			avg = (double)(r+g+b) / 3.0;
			hsiI[i][j] = avg;
		}
	}
}

//CHECK VALUE FOR I
int hsi::checkValue(int value) {

	if(value < BLACK)
		return BLACK;
	if(value > WHITE)
		return WHITE;
	return value;
}

//GET FUNCTIONS
vector< vector<double> > hsi::getHvec() {
	return hsiH;
}
vector< vector<double> > hsi::getSvec() {
	return hsiS;
}
vector< vector<double> > hsi::getIvec() {
	return hsiI;
}

double hsi::getH(int i, int j) {
	return hsiH[i][j];
}
double hsi::getS(int i, int j) {
	return hsiS[i][j];
}
int hsi::getI(int i, int j) {
	return hsiI[i][j];
}
string hsi::getFname(void) {
	return fname;
}
int hsi::getRows() {
	return rows;
}
int hsi::getCols() {
	return cols;
}


//STRETCH FUNCTIONS
void hsi::stretchH(pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size) {

	//int norm;
	pair<int,int> stop;
	double ratio, tempd;

	// stop.first = start.first + size.first;
	// stop.second = start.second + size.second;

	ratio = (double)(cd.second - cd.first) / (double)(ab.second - ab.first);

	//converts pixels using Inew = (d-c)/(b-a)[Iij - a] + c
	for(int i = start.second; i < size.second + start.second; ++i) {

		for(int j = start.first; j < size.first + start.first; ++j) {

			if(hsiH[i][j] < ab.first)
				hsiH[i][j] = MINHUE;

			else if(hsiH[i][j] > ab.second)
				hsiH[i][j] = MAXHUE;

			else {
				tempd = ratio * (double)(hsiH[i][j] - ab.first) + cd.first;
				
				hsiH[i][j] = tempd;
			}
		}
	}
}

void hsi::stretchS(pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size) {

	int norm;
	pair<int,int> stop;
	double ratio, tempd;

	// stop.first = start.first + size.first;
	// stop.second = start.second + size.second;

	ratio = ((double)cd.second - (double)cd.first) / ((double)ab.second - (double)ab.first);

	//converts pixels using Inew = (d-c)/(b-a)[Iij - a] + c
	for(int i = start.second; i < size.second + start.second; ++i) {

		for(int j = start.first; j < size.first + start.first; ++j) {
			
			norm = 255*hsiS[i][j];
			if(norm < ab.first)
				hsiS[i][j] = (double)BLACK/255.0;

			else if(norm > ab.second)
				hsiS[i][j] = (double)WHITE/255.0;

			else {
				tempd = ratio * (double)(norm - ab.first) + cd.first;
				
				hsiS[i][j] = tempd/255.0;
			}
		}
	}
}

void hsi::stretchI(pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size) {

	int temp;
	// pair<int,int> stop;
	double ratio, tempd, val;

	// stop.first = start.first + size.first;
	// stop.second = start.second + size.second;
	// cout << "start = " << start.first << ", " << start.second << endl;
	// cout << "stop = " << size.first + start.first << ", " << size.second + start.second << endl;
	// cout << "vec dim = " << hsiI.size() << ", " << hsiI[0].size() << endl;

	ratio = (double)(cd.second - cd.first) / (double)(ab.second - ab.first);
	// cout << "ratio = " << ratio << endl;
	//converts pixels using Inew = (d-c)/(b-a)[Iij - a] + c
	// int num_rows = hsiI.size(), num_cols = hsiI[0].size();
	// cout << "num_rows, num_cols = " << num_rows << ", " << num_cols << endl;
	for(int i = start.second; i < size.second + start.second; ++i) {
		
		for(int j = start.first; j < size.first + start.first; ++j) {
			val = hsiI[i][j];
			if(hsiI[i][j] < ab.first){
				hsiI[i][j] = cd.first;

			}

			if(hsiI[i][j] > ab.second){
				hsiI[i][j] = cd.second;

			}

			else {
				tempd = ratio * (double)(val - ab.first) + cd.first;
				temp = tempd;
				// cout << "i, j = " << i << ", " << j << endl;
				hsiI[i][j] = temp;
			}
		}
	}
	// cout << "num_rows, num_cols = " << num_rows << ", " << num_cols << endl;
	// cout << "min, max = " << minn << ", " << maxx << endl;
}

void hsi::stretchHSI(pair<int,int> ab, pair<int,int> cd, pair<int,int> start, pair<int,int> size) {

	this->stretchH(ab,cd,start,size);
	this->stretchS(ab,cd,start,size);
	this->stretchI(ab,cd,start,size);
}

