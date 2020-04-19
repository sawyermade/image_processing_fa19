# Image Processing HW4
## Compile
```bash
# Creates binary called hw4
bash ./compile.sh
```

## Run
```bash
# Runs with params.txt
./hw4 params.txt
```

## Compile & Run
```bash
# Compiles and runs with params.txt
bash ./run.sh
```

## Parameters
### Histogram Equalization
```
# path/to/input    path/to/output            #rois hist_equal X Y W   H
images/crayons.ppm output/crayons-hist_equal.png 1 hist_equal 0 0 600 480
```

### Histogram Stretching
```
# path/to/input    path/to/output            #rois hist_stretch X Y W   H   Min Max
images/crayons.ppm output/crayons-hist_equal.png 2 hist_stretch 0 0 600 480 100 255
```

### Sobel
```
# path/to/input    path/to/output       #rois sobel X Y W   H
images/crayons.ppm output/crayons-sobel.png 1 sobel 0 0 600 480
```

### Canny
```
# path/to/input    path/to/output       #rois canny X Y W   H
images/crayons.ppm output/crayons-canny.png 1 canny 0 0 600 480
```

### Sobel with Histogram
```
# path/to/input    path/to/output       #rois sobel_hist X Y W   H
images/crayons.ppm output/crayons-sobel_hist.png 1 sobel_hist 0 0 600 480
```

### Canny with Histogram
```
# path/to/input    path/to/output       #rois canny_hist X Y W   H
images/crayons.ppm output/crayons-canny_hist.png 1 canny_hist 0 0 600 480
```

