#include "image.h"
#include "utilities.h"
#include <strings.h>
#include <string.h>

//MY INCLUDES
#include <fstream>
#include "hsi.h"

using namespace std;

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

	//attempts to open parameters file.
	fp.open(argv[1],fstream::in);
	if(!fp.is_open()) {
		cout << "\nCant open parameters file: " << argv[1] << endl;
		return -1;
	}

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

		//gets src image filename.
		fp >> infile;
		
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

			//grayscale binarization/threshold process.
			if(!strncasecmp(pch.c_str(),"bin",MAXLEN)) {
				
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

			// Optimal Thresholding
			else if(!strncasecmp(pch.c_str(),"oth",MAXLEN)) {
				// Stuff
				int thresh_opt;

				if(!ovlap){
					thresh_opt = utilities::optimalThreshGS(src, tgt, start, size);
					cout << "thresh_opt = " << thresh_opt << endl;
				}
			}

			// No valid function
			else {
				cout << "\nNo Function: " << pch << endl;
				break;
			}
		}

		// Saves target
		tgt.save();
		getline(fp,strtemp);
	}

	// Closes file and calls it a day
	fp.close();
	return 0;
}
