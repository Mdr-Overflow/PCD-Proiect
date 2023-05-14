#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

//command:  gcc -o bmp2jpeg bmp2jpeg.c -ljpeg && ./bmp2jpeg asus.bmp

int convertBMPtoJPEG(const char *inputFilename, const char *outputFilename, int quality) {
  // Open the BMP input file
  FILE *inputFile = fopen(inputFilename, "rb");
  if (!inputFile) {
    fprintf(stderr, "Error opening input file: %s\n", inputFilename);
    return 0;
  }

  // Read BMP header
  unsigned char bmpHeader[54];
  fread(bmpHeader, sizeof(unsigned char), 54, inputFile);

  // Extract image dimensions from the BMP header
  int width = *(int *)&bmpHeader[18];
  int height = *(int *)&bmpHeader[22];
  int bitsPerPixel = *(int *)&bmpHeader[28];

  // BMP stores the image data in BGR format, so we need to determine the number of channels
  int numChannels = bitsPerPixel / 8;

  // BMP uses 4-byte aligned rows, so calculate the padding bytes
  int paddingBytes = (4 - (width * numChannels) % 4) % 4;

  // Calculate the size of the pixel data
  int imageSize = (width * numChannels + paddingBytes) * height;

  // Allocate memory for the pixel data
  unsigned char *imageData = (unsigned char *)malloc(imageSize);

  // Read the pixel data from the BMP file
  fread(imageData, sizeof(unsigned char), imageSize, inputFile);

  // Close the BMP file
  fclose(inputFile);

  // Create the JPEG output file
  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    free(imageData);
    return 0;
  }

  // Create the JPEG compression structure
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  // Set up error handling
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // Set the output file
  jpeg_stdio_dest(&cinfo, outputFile);

  // Set image information
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = numChannels;
  cinfo.in_color_space = JCS_RGB;

  // Set default compression parameters
  jpeg_set_defaults(&cinfo);

  // Set the compression quality
  jpeg_set_quality(&cinfo, quality, TRUE);

  // Start compression
  jpeg_start_compress(&cinfo, TRUE);

  // Create an array of scanlines
  JSAMPROW row_pointer[1];
  row_pointer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * width * numChannels);

  // Write scanlines
  for (int y = 0; y < height; y++) {
    // Copy a scanline from imageData to row_pointer
    int scanline = y;
    for (int x = 0; x < width; x++) {
      for (int c = 0; c < numChannels; c++) {
        row_pointer[0][x * numChannels + c] =
            imageData[scanline * (width * numChannels + paddingBytes) +
                      x * numChannels + c];
      }
    }
    // Write the scanline
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  // Finish compression
 
jpeg_finish_compress(&cinfo);

// Clean up the JPEG compression structure
jpeg_destroy_compress(&cinfo);

// Close the output file
fclose(outputFile);

// Free memory
free(imageData);
free(row_pointer[0]);

printf("Conversion completed successfully.\n");

return 1;
}

int main(int argc, char **argv) {
if (argc != 2) {
printf("Usage: %s input.bmp\n", argv[0]);
return 1;
}

const char *inputFilename = argv[1];
const char *outputFilename = "output.jpg";

int quality = 90;

if (convertBMPtoJPEG(inputFilename, outputFilename, quality)) {
printf("BMP to JPEG conversion successful!\n");
} else {
printf("BMP to JPEG conversion failed.\n");
}

return 0;
}