#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

//command: gcc jpeg_blurr.c -o jpeg_blurr -ljpeg && ./jpeg_blurr saturn.jpeg


typedef struct {
  unsigned char red, green, blue;
} Pixel;

void blurJPG(const char *inputFile, const char *outputFile) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_compress_struct outcinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  FILE *inputFileStream = NULL;
  FILE *outputFileStream = NULL;
  unsigned char *imageBuffer = NULL;
  unsigned char *blurredBuffer = NULL;

  cinfo.err = jpeg_std_error(&jerr);

  // Open the input file
  inputFileStream = fopen(inputFile, "rb");
  if (inputFileStream == NULL) {
    printf("Could not open input file.\n");
    return;
  }

  // Create the decompression struct
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, inputFileStream);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  // Allocate memory for the image buffer
  imageBuffer = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
  if (imageBuffer == NULL) {
    printf("Failed to allocate memory for image buffer.\n");
    jpeg_destroy_decompress(&cinfo);
    fclose(inputFileStream);
    return;
  }

  // Read the image data
  while (cinfo.output_scanline < cinfo.output_height) {
    row_pointer[0] = &imageBuffer[cinfo.output_scanline * cinfo.output_width * cinfo.output_components];
    jpeg_read_scanlines(&cinfo, row_pointer, 1);
  }

  // Create the compression struct
  outcinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&outcinfo);

  // Open the output file
  outputFileStream = fopen(outputFile, "wb");
  if (outputFileStream == NULL) {
    printf("Could not create output file.\n");
    jpeg_destroy_compress(&outcinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(inputFileStream);
    free(imageBuffer);
    return;
  }

  // Configure the output compression settings
  outcinfo.image_width = cinfo.output_width;
  outcinfo.image_height = cinfo.output_height;
  outcinfo.input_components = cinfo.output_components;
  outcinfo.in_color_space = cinfo.out_color_space;
  jpeg_stdio_dest(&outcinfo, outputFileStream);
  jpeg_set_defaults(&outcinfo);
  jpeg_set_quality(&outcinfo, 100, TRUE);
  jpeg_start_compress(&outcinfo, TRUE);

  // Allocate memory for the blurred image buffer
  blurredBuffer = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
  if (blurredBuffer == NULL) {
    printf("Failed to allocate memory for blurred buffer.\n");
    jpeg_destroy_compress(&outcinfo);
    jpeg_destroy_decompress(&  cinfo);
    fclose(outputFileStream);
    jpeg_destroy_decompress(&cinfo);
    fclose(inputFileStream);
    free(imageBuffer);
    return;
  }

  // Process the image pixel by pixel and blur it
  for (int y = 0; y < cinfo.output_height; y++) {
    for (int x = 0; x < cinfo.output_width; x++) {
      unsigned int totalRed = 0, totalGreen = 0, totalBlue = 0;
      unsigned int pixelCount = 0;

      // Sum the neighboring pixels' color channels
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          int currentX = x + j;
          int currentY = y + i;

          // Check if the neighboring pixel is within bounds
          if (currentX >= 0 && currentX < cinfo.output_width && currentY >= 0 && currentY < cinfo.output_height) {
            unsigned int index = (currentY * cinfo.output_width + currentX) * cinfo.output_components;
            totalRed += imageBuffer[index];
            totalGreen += imageBuffer[index + 1];
            totalBlue += imageBuffer[index + 2];
            pixelCount++;
          }
        }
      }

      // Calculate the average of the neighboring pixels
      unsigned int blurredRed = totalRed / pixelCount;
      unsigned int blurredGreen = totalGreen / pixelCount;
      unsigned int blurredBlue = totalBlue / pixelCount;

      // Set the blurred pixel values in the output buffer
      unsigned int outputIndex = (y * cinfo.output_width + x) * cinfo.output_components;
      blurredBuffer[outputIndex] = blurredRed;
      blurredBuffer[outputIndex + 1] = blurredGreen;
      blurredBuffer[outputIndex + 2] = blurredBlue;
    }
  }

  // Write the blurred image data to the output file
  for (int y = 0; y < cinfo.output_height; y++) {
    row_pointer[0] = &blurredBuffer[y * cinfo.output_width * cinfo.output_components];
    jpeg_write_scanlines(&outcinfo, row_pointer, 1);
  }

  // Finish writing the output file
  jpeg_finish_compress(&outcinfo);

  // Cleanup and release resources
  jpeg_destroy_compress(&outcinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(inputFileStream);
  fclose(outputFileStream);
  free(imageBuffer);
  free(blurredBuffer);

  printf("Image blurred successfully. Saved as %s\n", outputFile);
}

int blurJPG_main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  blurJPG(argv[1], "jpeg_blurr.jpg");

  return 0;
}

