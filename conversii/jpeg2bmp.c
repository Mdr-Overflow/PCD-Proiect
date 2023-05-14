#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <stdint.h>

int convertJPEGtoBMP(const char *inputFilename, const char *outputFilename) {
  // Open the JPEG input file
  FILE *inputFile = fopen(inputFilename, "rb");
  if (!inputFile) {
    fprintf(stderr, "Error opening input file: %s\n", inputFilename);
    return 0;
  }

  // Initialize the JPEG decompression object
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  // Set the source file
  jpeg_stdio_src(&cinfo, inputFile);

  // Read the JPEG header
  jpeg_read_header(&cinfo, TRUE);

  // Start the decompression
  jpeg_start_decompress(&cinfo);

  // Extract image dimensions
  int width = cinfo.output_width;
  int height = cinfo.output_height;
  int numChannels = cinfo.output_components;

  // Calculate row stride in bytes
  int rowStride = width * numChannels;

  // Allocate memory for the pixel data
  uint8_t *imageData = (uint8_t *)malloc(rowStride * height);

  // Read scanlines one by one
  while (cinfo.output_scanline < height) {
    uint8_t *rowPointer = imageData + (cinfo.output_scanline) * rowStride;
    jpeg_read_scanlines(&cinfo, &rowPointer, 1);
  }

  // Finish decompression
  jpeg_finish_decompress(&cinfo);

  // Clean up the JPEG decompression object
  jpeg_destroy_decompress(&cinfo);
  fclose(inputFile);

  // Create the BMP output file
  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    free(imageData);
    return 0;
  }

  // Set BMP file header
  uint16_t fileType = 0x4D42;
  fwrite(&fileType, sizeof(uint16_t), 1, outputFile);
  uint32_t fileSize = 54 + rowStride * height;
  fwrite(&fileSize, sizeof(uint32_t), 1, outputFile);
  uint32_t reserved = 0;
  fwrite(&reserved, sizeof(uint32_t), 1, outputFile);
  uint32_t dataOffset = 54;
  fwrite(&dataOffset, sizeof(uint32_t), 1, outputFile);

  // Set BMP image header
  uint32_t headerSize = 40;
  fwrite(&headerSize, sizeof(uint32_t), 1, outputFile);
  uint32_t imageWidth = width;
  fwrite(&imageWidth, sizeof(uint32_t), 1, outputFile);
  uint32_t imageHeight = height;
  fwrite(&imageHeight, sizeof(uint32_t), 1, outputFile);
  uint16_t planes = 1;
  fwrite(&planes, sizeof(uint16_t), 1, outputFile);
  uint16_t bitsPerPixel = numChannels * 8;
  fwrite(&bitsPerPixel, sizeof(uint16_t), 1, outputFile);
  uint32_t compression = 0;
  fwrite(&compression, sizeof(uint32_t), 1, outputFile);
  uint32_t imageSize = rowStride * height;
  fwrite(&imageSize, sizeof(uint32_t), 1, outputFile);
  uint32_t xPixelsPerMeter = 0;
  fwrite(&xPixelsPerMeter, sizeof(uint32_t), 1, outputFile);
  uint32_t yPixelsPerMeter = 0;
  fwrite(&yPixelsPerMeter, sizeof(uint32_t), 1, outputFile);
  uint32_t totalColors = 0;
  fwrite(&totalColors, sizeof(uint32_t), 1, outputFile);
  uint32_t importantColors = 0;
  fwrite(&importantColors, sizeof(uint32_t), 1, outputFile);

  // Write the pixel data in BMP format (BGR)
  for (int y = height - 1; y >= 0; y--) {
    uint8_t *rowPointer = imageData + y * rowStride;
    for (int x = 0; x < width; x++) {
      uint8_t *pixel = rowPointer + x * numChannels;
      fwrite(pixel + 2, 1, 1, outputFile); // Blue channel
      fwrite(pixel + 1, 1, 1, outputFile); // Green channel
      fwrite(pixel, 1, 1, outputFile);     // Red channel
    }
    // Add padding bytes at the end of each row
    for (int p = 0; p < rowStride - width * numChannels; p++) {
      fputc(0, outputFile);
    }
  }

  // Clean up
  fclose(outputFile);
  free(imageData);

  printf("Conversion completed successfully.\n");

  return 1;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s input.jpg\n", argv[0]);
    return 1;
  }

  const char *inputFilename = argv[1];
  const char *outputFilename = "output.bmp";

  if (convertJPEGtoBMP(inputFilename, outputFilename)) {
    printf("JPEG to BMP conversion successful!\n");
  } else {
    printf("JPEG to BMP conversion failed.\n");
  }

  return 0;
}
