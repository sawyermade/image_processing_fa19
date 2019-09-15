#include <iostream>
#include <vector>

using namespace std;

int main(){

	vector<vector<int> > rois_vec;
	vector<int> roi_vec;

	for(int i = 0; i < 10; i++){
		roi_vec.push_back(i);
		roi_vec.push_back(i+1);
		roi_vec.push_back(i+2);
		roi_vec.push_back(i+3);

		rois_vec.push_back(roi_vec);
		roi_vec.clear();
	}

	for(int i = 0; i < rois_vec.size(); i++){

		for(int j = 0; j < rois_vec[i].size(); j++){
			cout << rois_vec[i][j] << " ";
		}
		cout << endl;
	}

	int m, v;
	for(m = 10, v = 10; m > 0; m--){
		if(v < m)
			continue;
		else
			break;
	}
	cout << endl;
	cout << m << endl;
}