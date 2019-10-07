import cv2, numpy as np, sys, os
from matplotlib import pyplot as plt


if __name__ == '__main__':

	img_path = sys.argv[1]
	img = cv2.imread(img_path, -1)
	img_he = np.copy(img)

	rk, nk = np.unique(img, return_counts=True)
	pk = nk / img.size
	
	sk = np.cumsum(pk)
	mul = sk * 255
	map_array = np.round(mul)

	for i in range(img.shape[0]):
		for j in range(img.shape[1]):
			pix_val = img[i, j]
			map_indice = np.where(rk == pix_val)
			pix_mapped = map_array[map_indice]
			img_he[i, j] = pix_mapped

	cv2.imwrite('temp.pgm', img_he)

	plt.hist(img.ravel(), 256, [0, 255])
	plt.show()

	plt.hist(img_he.ravel(), 256, [0, 255])
	plt.show()