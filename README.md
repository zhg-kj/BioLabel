# BioLabel
A tool for preprocessing bioinformatics image data. Enables researchers to efficiently label images. Includes the [BioStitch](https://github.com/zhg-kj/BioStitch) software. Developed by Kai Jun Zhuang.

## Usage
To use the application, download the deployed version from https://www.dropbox.com/sh/2zhn6qsganposnx/AACSZFWr6mPQxTXr-VC8ahiya?dl=0, extract the zip folder into your desired folder, open the folder and click on the BioStitch.exe file to open the application.

Click on file and then stitch images. You will be prompted to select two folders. The first folder you are prompted to select is the folder that contains the subfolders with naming convention XY. These subfolders should contain the images that need to be stitched. The second folder you are prompted select is where all the stitched images will be saved to.

Note that this tool makes several assumptions:
1. The folder you choose contains subfolders that contain the images to be stitched. These subfolders must contain XY in the name.
2. The subfolders contain 4 channels (naming convention CH1, CH2, CH3, CH4) and an overlay (naming convention Overlay). There must be 9 images for each channel/overlay. These 9 images will be stitched together to create the stitched image.
3. Stitched images are in a .tif file format.
4. The 9 images are numbered in their file name so that they are ordered. The numbering of these images determine where they will be stitched.

Below is how the order of the images will be stitched together. If you make no changes to the names of the images, it should be correct by default.

```
1 2 3 
6 5 4 
7 8 9
```

Below is a tutorial of how to use this tool.

https://user-images.githubusercontent.com/67027837/209880391-b27aabf6-9479-481a-8fdb-3d029085b843.mp4

## Contribute
Install Qt Creator, clone repository, open repository in Qt creator.

## Troubleshooting
If you receive errors related to QPixmap or QImage, it is highly likely that your installation did not include the plugin that allows Qt to handle .tif files. Navigate to your installation directory and ensure that plugins > imageformats includes a file named qtiff.dll
