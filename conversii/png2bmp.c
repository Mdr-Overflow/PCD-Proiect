#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>

// Function to convert PNG to BMP
int convertPNGtoBMP(const char *inputFilename, const char *outputFilename) {
  // Open the PNG input file
  FILE *inputFile = fopen(inputFilename, "rb");
  if (!inputFile) {
    fprintf(stderr, "Error opening input file: %s\n", inputFilename);
    return 0;
  }

  // Initialize the PNG structures
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fprintf(stderr, "Error initializing PNG read structure\n");
    fclose(inputFile);
    return 0;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    fprintf(stderr, "Error initializing PNG info structure\n");
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(inputFile);
    return 0;
  }

  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "Error during PNG read\n");
    png_destroy_read_struct(&png, &info, NULL);
    fclose(inputFile);
    return 0;
  }

  png_init_io(png, inputFile);
  png_read_info(png, info);

  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  int color_type = png_get_color_type(png, info);

  int numChannels = 3;
  if (color_type == PNG_COLOR_TYPE_GRAY) {
    numChannels = 1;
  } else if (color_type == PNG_COLOR_TYPE_RGB) {
    numChannels = 3;
  } else if (color_type == PNG_COLOR_TYPE_RGBA) {
    numChannels = 4;
  } else {
    fprintf(stderr, "Unsupported color type\n");
    png_destroy_read_struct(&png, &info, NULL);
    fclose(inputFile);
    return 0;
  }

  png_read_update_info(png, info);

  png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
  }

  png_read_image(png, row_pointers);
  png_destroy_read_struct(&png, &info, NULL);
  fclose(inputFile);

  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    return 0;
  }

  int bytesPerRow = width * numChannels;
  int paddingBytes = (4 - (bytesPerRow % 4)) % 4;
  int totalBytesPerRow = bytesPerRow + paddingBytes;
  int fileSize = 54 + totalBytesPerRow * height;

  unsigned char bmpHeader[54] = {
      'B', 'M',
      fileSize & 0xFF, (fileSize >> 8) & 0xFF, (fileSize >> 16) & 0xFF, fileSize >> 24,
      0, 0, 0, 0,
      54, 0, 0, 0,
      40, 0, 0, 0,
      width & 0xFF, (width >> 8) & 0xFF, (width >> 16) & 0xFF, width >> 24,
      height & 0xFF, (height >> 8) & 0xFF, (height >> 16) & 0xFF, height >> 24,
      1, 0,
      numChannels * 8, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
  };

  fwrite(bmpHeader, sizeof(unsigned char), 54, outputFile);

  for (int y = height - 1; y >= 0; y--) {
  // Write the image data with channel ordering modification
  for (int x = 0; x < width; x++) {
    png_bytep pixel = &(row_pointers[y][x * numChannels]);

    // Swap blue and red channels (assuming BGR channel ordering)
    png_byte blue = pixel[0];
    png_byte red = pixel[2];
    pixel[0] = red;
    pixel[2] = blue;
  }

  fwrite(row_pointers[y], sizeof(png_byte), bytesPerRow, outputFile);

  // Add padding bytes
  for (int p = 0; p < paddingBytes; p++) {
    fputc(0, outputFile);
  }
}


  fclose(outputFile);

  // Free memory
  for (int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  printf("Conversion completed successfully.\n");

  return 1;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s input.png\n", argv[0]);
    return 1;
  }

  const char *inputFilename = argv[1];
  const char *outputFilename = "output.bmp";

  if (convertPNGtoBMP(inputFilename, outputFilename)) {
    printf("PNG to BMP conversion successful!\n");
  } else {
    printf("PNG to BMP conversion failed.\n");
  }

  return 0;
}
