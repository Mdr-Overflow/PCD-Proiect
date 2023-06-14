#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>

//gcc bmp2png.c -o bmp2png -lpng && ./bmp2png asus.bmp

int convertBMPtoPNG(const char *inputFilename, const char *outputFilename) {
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

  // BMP stores the image data in BGR format, so we need to determine the number
  // of channels
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

  // Create the PNG output file
  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    free(imageData);
    return 0;
  }

  // Create the PNG write structure and info structure
  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fprintf(stderr, "Error initializing PNG write structure\n");
    fclose(outputFile);
    free(imageData);
    return 0;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    fprintf(stderr, "Error initializing PNG info structure\n");
    png_destroy_write_struct(&png, NULL);
    fclose(outputFile);
    free(imageData);
    return 0;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "Error during PNG write\n");
    png_destroy_write_struct(&png, &info);
    fclose(outputFile);
    free(imageData);
    return 0;
  }

  // Set up PNG IO
  png_init_io(png, outputFile);

  // Set the PNG image information
  png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // Write the PNG header
  png_write_info(png, info);

  // Create row pointers
  png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    row_pointers[y] = &(imageData[(width * numChannels + paddingBytes) * y]);
  }

  // Write the PNG image data (with 180-degree rotation)
  for (int y = height - 1; y >= 0; y--) {
    png_write_row(png, row_pointers[y]);
  }

  // Write the end of the PNG file
  png_write_end(png, NULL);

  // Clean up
  png_destroy_write_struct(&png, &info);
  fclose(outputFile);
  free(imageData);
  free(row_pointers);

  printf("Conversion completed successfully.\n");

  return 1;
}

int convertBMPtoPNG_main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s input.bmp\n", argv[0]);
    return 1;
  }

  const char *inputFilename = argv[1];
  const char *outputFilename = "output.png";

  if (convertBMPtoPNG(inputFilename, outputFilename)) {
    printf("BMP to PNG conversion successful!\n");
  } else {
    printf("BMP to PNG conversion failed.\n");
  }

  return 0;
}
