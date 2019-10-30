import os, subprocess

process_list = []
for file in os.listdir('./'):
	if file.endswith('.ppm') or file.endswith('.pgm'):
		fname, ext = file.rsplit('.', 1)
		outfile = f'{fname}.jpg'
		cmd = ['convert', file, outfile]
		subprocess.run(cmd)