# Image Processing Grad USF Fall 2019 HW4
## Processes:
dftlp = DFT with low pass filter

dfthp = DFT with high pass filter

dftn = DFT with notch filter

dftlpn = DFT with low pass filter and notch

dfthpbp = DFT with high pass filter and band pass

## Parameters for Processes:
dftlp X Y Sx Sy D0

dfthp X Y Sx Sy D0

dftn X Y Sx Sy D1 D2

dftlpn X Y Sx Sy D0 D1 D2

dfthpbp X Y Sx Sy D0 D1 D2

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
cd image_processing_fa19/hw4
make
./hw parameters.txt
```

## Parameters file
```bash
cat parameters.txt
```
