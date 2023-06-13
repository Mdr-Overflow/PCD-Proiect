#include <png.h>
#include <stdio.h>
#include <stdlib.h>

//command: gcc -o png_grayscale png_grayscale.c -lpng && ./png_grayscale dog.png



void grayOutPNG(const char *inputFileName, const char *outputFileName) {
  // Open the input file
  FILE *inputFile = fopen(inputFileName, "rb");
  if (!inputFile) {
    printf("Error: Failed to open the input file.\n");
    return;
  }

  // Create a read structure for libpng
  png_structp pngReadStruct =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngReadStruct) {
    printf("Error: Failed to create the PNG read structure.\n");
    fclose(inputFile);
    return;
  }

  // Create an info structure for libpng
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

  // Update the color type and bit depth to RGBA 8-bit if necessary
  if (colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(pngReadStruct);
  if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    png_set_expand_gray_1_2_4_to_8(pngReadStruct);
  if (png_get_valid(pngReadStruct, pngInfoStruct, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pngReadStruct);

  if (bitDepth == 16)
    png_set_strip_16(pngReadStruct);

  if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY ||
      colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(pngReadStruct, 0xFF, PNG_FILLER_AFTER);

  if (colorType == PNG_COLOR_TYPE_GRAY ||
      colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(pngReadStruct);

  // Update the info structure
  png_read_update_info(pngReadStruct, pngInfoStruct);

  // Allocate memory for image data
  png_bytep *rowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (png_uint_32 y = 0; y < height; ++y)
    rowPointers[y] =
        (png_byte *)malloc(png_get_rowbytes(pngReadStruct, pngInfoStruct));

  // Read image data
  png_read_image(pngReadStruct, rowPointers);

  // Calculate the row size for the grayscale image
  png_uint_32 grayRowSize = width * 3; // 3 bytes per pixel (RGB)
  // Allocate memory for the grayscale image data
  png_bytep *grayRowPointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (png_uint_32 y = 0; y < height; ++y)
    grayRowPointers[y] = (png_byte *)malloc(grayRowSize);

  // Perform grayscale conversion
  for (png_uint_32 y = 0; y < height; ++y) {
    png_bytep row = rowPointers[y];
    png_bytep grayRow = grayRowPointers[y];
    for (png_uint_32 x = 0; x < width; ++x) {
      png_bytep pixel = &(row[x * 4]);
      png_bytep grayPixel = &(grayRow[x * 3]);
      png_byte gray =
          (png_byte)(0.21 * pixel[0] + 0.72 * pixel[1] + 0.07 * pixel[2]);
      grayPixel[0] = grayPixel[1] = grayPixel[2] = gray;
    }
  }

  // Open the output file
  FILE *outputFile = fopen(outputFileName, "wb");
  if (!outputFile) {
    printf("Error: Failed to open the output file.\n");
    for (png_uint_32 y = 0; y < height; ++y) {
      free(rowPointers[y]);
      free(grayRowPointers[y]);
    }
    free(rowPointers);
    free(grayRowPointers);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Create a write structure for libpng
  png_structp pngWriteStruct =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngWriteStruct) {
    printf("Error: Failed to create the PNG write structure.\n");
    for (png_uint_32 y = 0; y < height; ++y) {
      free(rowPointers[y]);
      free(grayRowPointers[y]);
    }
    free(rowPointers);
    free(grayRowPointers);
    fclose(outputFile);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Set up error handling
  if (setjmp(png_jmpbuf(pngWriteStruct))) {
    printf("Error: An error occurred while writing the PNG file.\n");
    for (png_uint_32 y = 0; y < height; ++y) {
      free(rowPointers[y]);
      free(grayRowPointers[y]);
    }
    free(rowPointers);
    free(grayRowPointers);
    png_destroy_write_struct(&pngWriteStruct, NULL);
    fclose(outputFile);
    png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
    fclose(inputFile);
    return;
  }

  // Initialize libpng I/O
  png_init_io(pngWriteStruct, outputFile);

  // Set image information
  png_set_IHDR(pngWriteStruct, pngInfoStruct, width, height, 8,
               PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  // Write image data
  png_write_info(pngWriteStruct, pngInfoStruct);
  png_write_image(pngWriteStruct, grayRowPointers);
  png_write_end(pngWriteStruct, NULL);

  // Clean up
  for (png_uint_32 y = 0; y < height; ++y) {
    free(rowPointers[y]);
    free(grayRowPointers[y]);
  }
  free(rowPointers);
  free(grayRowPointers);

  // Finalize and close files
  png_destroy_write_struct(&pngWriteStruct, NULL);
  png_destroy_read_struct(&pngReadStruct, &pngInfoStruct, NULL);
  fclose(outputFile);
  fclose(inputFile);

  printf("The PNG image has been grayed out successfully.\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Error: No input file specified.\n");
    return 1;
  }

  const char *inputFileName = argv[1];
  const char *outputFileName = "png_grayscaled.png";
  grayOutPNG(inputFileName, outputFileName);

  return 0;
}
