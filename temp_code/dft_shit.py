import numpy as np, cv2, sys, os
from matplotlib import pyplot as plt

if __name__ == '__main__':
	# Get image path and open
	img_path = sys.argv[1]
	img = cv2.imread(img_path, 0)
	img = np.float32(img)

	# Run DFT
	dft = cv2.dft(img, flags=cv2.DFT_COMPLEX_OUTPUT)
	dft_shift = np.fft.fftshift(dft)

	# Find min and max
	min_dft , max_dft = 1000000000, -1000000000
	for i in range(dft.shape[0]):
		for j in range(dft.shape[1]):
			if dft