# Image Processing Grad USF Fall 2019 HW3
## Processes:
sobel = runs sobel filter over ROI

sobelgb = sobel with amplitude/gradient binarization

sobelgbdb = sobel with amplitude/gradient binarization and degree/direction binarization

OPENCV Processes. Cannot run opencv and non-opencv processes on the same image

ocvsobel = OpenCV sobel implementation

ocvcanny = OpenCV canny

ocvhist = OpenCV histogram equalization

ocvotsu = OpenCV OTSU

ocvotsuhist = OpenCV OTSU with foreground histogram equalized

## Parameters for Processes: WS is odd number that will be squared for total size
sobel X Y Sx Sy WS

sobel X Y Sx Sy WS Ta

sobelgbdb X Y Sx Sy WS Ta Td

ocvsobel X Y Sx Sy WS

ocvcanny X Y Sx Sy WS

ocvhist X Y Sx Sy 

ocvotsu X Y Sx Sy 

ocvotsuhist X Y Sx Sy

## Input File Format:
input/file output/file #Processes ProcessName Parameters
### Example:
```
          Input                  Output                  #P    P       X  Y   Sx  Sy
sample-images/baboon.ppm output-images/baboon_roi12ds.ppm 1 histgsoth 50 100 150 300
```

## Compile and Run:
Output images written in output-images/ directory after execution
```bash
# Clone from github if needed
git clone https://github.com/sawyermade/image_processing_fa19.git 
cd image_processing_fa19/hw2
make
./hw parameters.txt
```

## Parameters file
```bash
cat parameters.txt
```
