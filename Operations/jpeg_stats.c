#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void WriteJpegStatsToFile(const char *inputFile, const char *outputFile) {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *inputFileStream = NULL;
  FILE *outputFileStream = NULL;
  JSAMPARRAY buffer;
  int rowStride;

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

  // Get image resolution
  int width = cinfo.output_width;
  int height = cinfo.output_height;

  // Open the output file
  outputFileStream = fopen(outputFile, "w");
  if (outputFileStream == NULL) {
    printf("Could not open output file.\n");
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(inputFileStream);
    return;
  }

  // Write the resolution to the output file
  fprintf(outputFileStream, "Resolution: %dx%d\n", width, height);

  // Allocate memory for the buffer
  rowStride = cinfo.output_width * cinfo.output_components;
  buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
  for (int i = 0; i < cinfo.output_height; i++) {
    buffer[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * rowStride);
  }

  // Read the image data
  while (cinfo.output_scanline < cinfo.output_height) {
    int row = cinfo.output_scanline;
    jpeg_read_scanlines(&cinfo, &buffer[row], 1);
  }

  // Calculate the dominant color
  int redSum = 0, greenSum = 0, blueSum = 0;
  int pixelCount = width * height;
  for (int y = 0; y < height; y++) {
    JSAMPROW row = buffer[y];
    for (int x = 0; x < width; x++) {
      int offset = x * cinfo.output_components;
      redSum += row[offset];
      greenSum += row[offset + 1];
      blueSum += row[offset + 2];
    }
  }

  int red = redSum / pixelCount;
  int green = greenSum / pixelCount;
  int blue = blueSum / pixelCount;

  // Write the dominant color to the output file
  fprintf(outputFileStream, "Dominant Color: R=%d, G=%d, B=%d\n", red, green, blue);

  // Cleanup and release resources
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(inputFileStream);
  fclose(outputFileStream);

  // Free the buffer memory
  for (int i = 0; i < cinfo.output_height; i++) {
    free(buffer[i]);
  }
  free(buffer);
}

int WriteJpegStatsToFile_main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file> \n", argv[0]);
    return 1;
  }

  const char *inputFile = argv[1];
  const char *outputFile = "jpeg_stats.txt";

  WriteJpegStatsToFile(inputFile, outputFile);

  return 0;
}
