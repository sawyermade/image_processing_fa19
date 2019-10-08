import cv2, numpy as np, sys, os
from matplotlib import pyplot as plt


if __name__ == '__main__':
	# Opens img and copies it
	img_path = sys.argv[1]
	img = cv2.imread(img_path, -1)
	img_he = np.copy(img)

	# Get unique vals, freq of vals, and gets percentages of freq/total_pixels
	rk, nk = np.unique(img, return_counts=True)
	pk = nk / img.size
	print(pk)
	
	# Gets communative sum of percentages ascending order sorted
	# Multiplies sk by max_intensity and rounds to integer
	max_intensity = 255
	sk = np.cumsum(pk)
	mul = sk * max_intensity
	map_array = np.round(mul)
	print(sk)

	# Remaps every pixel and saves to new image
	for i in range(img.shape[0]):
		for j in range(img.shape[1]):
			pix_val = img[i, j]
			map_indice = np.where(rk == pix_val)
			pix_mapped = map_array[map_indice]
			img_he[i, j] = pix_mapped

	# Writes HE image
	cv2.imwrite('temp.pgm', img_he)

	# # Plots before histogram
	# plt.hist(img.ravel(), 256, [0, 255])
	# plt.show()

	# # Plots after historgram
	# plt.hist(img_he.ravel(), 256, [0, 255])
	# plt.show()