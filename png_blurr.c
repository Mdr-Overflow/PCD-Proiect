#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#define BLUR_RADIUS 1

//command: gcc -o png_blurr png_blurr.c -lpng && ./png_blurr linux.png

void blurPNG(const char *inputFileName, const char *outputFileName) {
  // Open the input file
  FILE *inputFile = fopen(inputFileName, "rb");
  if (!inputFile) {
    printf("Error: Failed to open the input file.\n");
    return;
  }

  // Create read structures for libpng
  png_structp pngReadStruct =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngReadStruct) {
    printf("Error: Failed to create the PNG read structure.\n");
    fclose(inputFile);
    return;
  }

  png_infop pngInfoStruct = png_create_info_struct(pngReadStruct);
  if (!pngInfoStruct) {
    printf("Error: Failed to create the PNG info structure.\n");
    png_destroy_read_struct(&pngReadStruct, NULL, NULL);
    fclose(inputFile);
    return;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(pngReadStruct))) {
    printf("Error: An error occurred while reading the PNG file.\n");
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Initialize libpng I/O
  png_init_io(pngReadStruct, inputFile);
  png_read_info(pngReadStruct, pngInfoStruct);

  // Get image attributes
  png_uint_32 width = png_get_image_width(pngReadStruct, pngInfoStruct);
  png_uint_32 height = png_get_image_height(pngReadStruct, pngInfoStruct);
  int colorType = png_get_color_type(pngReadStruct, pngInfoStruct);
  int bitDepth = png_get_bit_depth(pngReadStruct, pngInfoStruct);

  // Allocate memory for image data
  png_bytep *rowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (png_uint_32 y = 0; y < height; ++y)
    rowPointers[y] =
        (png_byte *)malloc(png_get_rowbytes(pngReadStruct, pngInfoStruct));

  // Read image data
  png_read_image(pngReadStruct, rowPointers);

  // Perform blurring
  for (png_uint_32 y = BLUR_RADIUS; y < height - BLUR_RADIUS; ++y) {
    png_bytep currentRow = rowPointers[y];
    png_bytep upperRow = rowPointers[y - BLUR_RADIUS];
    png_bytep lowerRow = rowPointers[y + BLUR_RADIUS];

    for (png_uint_32 x = BLUR_RADIUS; x < width - BLUR_RADIUS; ++x) {
      png_bytep currentPixel = &(currentRow[x * 4]);
      png_bytep leftPixel = &(currentRow[(x - BLUR_RADIUS) * 4]);
      png_bytep rightPixel = &(currentRow[(x + BLUR_RADIUS) * 4]);
      png_bytep upperPixel = &(upperRow[x * 4]);
      png_bytep lowerPixel = &(lowerRow[x * 4]);

      // Calculate average color values
      png_byte avgR = (leftPixel[0] + currentPixel[0] + rightPixel[0] +
                       upperPixel[0] + lowerPixel[0]) /
                      5;
      png_byte avgG = (leftPixel[1] + currentPixel[1] + rightPixel[1] +
                       upperPixel[1] + lowerPixel[1]) /
                      5;
      png_byte avgB = (leftPixel[2] + currentPixel[2] + rightPixel[2] +
                       upperPixel[2] + lowerPixel[2]) /
                      5;
      // Update current pixel with the average color values
      currentPixel[0] = avgR;
      currentPixel[1] = avgG;
      currentPixel[2] = avgB;
    }
  }

  // Open the output file
  FILE *outputFile = fopen(outputFileName, "wb");
  if (!outputFile) {
    printf("Error: Failed to open the output file.\n");
    for (png_uint_32 y = 0; y < height; ++y)
      free(rowPointers[y]);
    free(rowPointers);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Create write structures for libpng
  png_structp pngWriteStruct =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngWriteStruct) {
    printf("Error: Failed to create the PNG write structure.\n");
    for (png_uint_32 y = 0; y < height; ++y)
      free(rowPointers[y]);
    free(rowPointers);
    fclose(outputFile);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(pngWriteStruct))) {
    printf("Error: An error occurred while writing the PNG file.\n");
    for (png_uint_32 y = 0; y < height; ++y)
      free(rowPointers[y]);
    free(rowPointers);
    png_destroy_write_struct(&pngWriteStruct, NULL);
    fclose(outputFile);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Initialize libpng I/O
  png_init_io(pngWriteStruct, outputFile);

  // Set image information
  png_set_IHDR(pngWriteStruct, pngInfoStruct, width, height, bitDepth,
               colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // Write image data
  png_write_info(pngWriteStruct, pngInfoStruct);
  png_write_image(pngWriteStruct, rowPointers);
  png_write_end(pngWriteStruct, NULL);

  // Clean up
  for (png_uint_32 y = 0; y < height; ++y)
    free(rowPointers[y]);
  free(rowPointers);

  // Finalize and close files
  png_destroy_write_struct(&pngWriteStruct, NULL);
  png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
  fclose(outputFile);
  fclose(inputFile);

  printf("The PNG image has been blurred successfully.\n");
}

int blurPNG_main(int argc, char **argv) {
  const char *inputFileName = argv[1];
  const char *outputFileName = "blurred_output.png";
  blurPNG(inputFileName, outputFileName);

  if (argc < 2) {
    printf("Error: No input file specified.\n");
    return 1;
  }
  return 0;
}