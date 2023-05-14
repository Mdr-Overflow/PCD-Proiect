#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>


//command: gcc jpeg2png.c -o jpeg2png -ljpeg -lpng && ./jpeg2png test.jpeg
// Function to convert JPEG to PNG
int convertJPEGtoPNG(const char *inputFilename, const char *outputFilename) {
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

  // Set the input file
  jpeg_stdio_src(&cinfo, inputFile);

  // Read the JPEG header
  jpeg_read_header(&cinfo, TRUE);

  // Start the decompression
  jpeg_start_decompress(&cinfo);

  // Get the image dimensions
  int width = cinfo.output_width;
  int height = cinfo.output_height;
  int channels = cinfo.output_components;

  // Calculate the row size
  int row_stride = width * channels;

  // Allocate memory for the pixel buffer
  JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
  for (int i = 0; i < height; i++) {
    buffer[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);
  }

  // Read the scanlines of the JPEG image
  while (cinfo.output_scanline < height) {
    jpeg_read_scanlines(&cinfo, &buffer[cinfo.output_scanline],
                        height - cinfo.output_scanline);
  }

  // Finish decompression
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(inputFile);

  // Open the PNG output file
  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    return 0;
  }

  // Initialize the PNG write structure
  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fprintf(stderr, "Error initializing PNG write structure\n");
    fclose(outputFile);
    return 0;
  }

  // Initialize the PNG info structure
  png_infop info = png_create_info_struct(png);
  if (!info) {
    fprintf(stderr, "Error initializing PNG info structure\n");
    png_destroy_write_struct(&png, NULL);
    fclose(outputFile);
    return 0;
  }

  // Set the output file
  png_init_io(png, outputFile);

  // Set the image information
  png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  // Write the PNG header
  png_write_info(png, info);

  // Write the image data
  for (int y = 0; y < height; y++) {
    png_write_row(png, (png_bytep)buffer[y]);
  }

  // Finish writing
  png_write_end(png, NULL);

  // Clean up
  png_destroy_write_struct(&png, &info);
  fclose(outputFile);

  // Free the pixel buffer memory
  for (int i = 0; i < height; i++) {
    free(buffer[i]);
  }
  free(buffer);

  printf("Conversion completed successfully.\n");

  return 1;
}

int main(int argc, char** argv) {
  const char *inputFilename = argv[1];
  const char *outputFilename = "output.png";

  if (convertJPEGtoPNG(inputFilename, outputFilename)) {
    printf("JPEG to PNG conversion successful!\n");
  } else {
    printf("JPEG to PNG conversion failed.\n");
  }

  return 0;
}