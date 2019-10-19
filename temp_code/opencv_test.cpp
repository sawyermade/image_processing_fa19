#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

	Mat img;
	img = imread(string(argv[1]), -1);

	cout << "img.size() = " << img.size() << endl;

	return 0;
}