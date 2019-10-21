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
		tgt.save();
		getline(fp,strtemp);

	}

	// Closes file and calls it a day
	fp.close();
	return 0;
}
