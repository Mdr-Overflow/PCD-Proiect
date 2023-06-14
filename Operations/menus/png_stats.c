#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} Pixel;

void extractPNGStats(const char *filename, const char *outputFilename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    printf("Could not open the PNG file.\n");
    return;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    printf("Error creating PNG read struct.\n");
    fclose(file);
    return;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    printf("Error creating PNG info struct.\n");
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(file);
    return;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    printf("Error during PNG file read.\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);
    return;
  }

  png_init_io(png_ptr, file);
  png_read_info(png_ptr, info_ptr);

  int32_t width = png_get_image_width(png_ptr, info_ptr);
  int32_t height = png_get_image_height(png_ptr, info_ptr);
  printf("Resolution: %dx%d\n", width, height);

  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA) {
    printf("Unsupported PNG color type.\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);
    return;
  }

  png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  if (!row_pointers) {
    printf("Memory allocation failed.\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);
    return;
  }

  uint32_t i, j;
  for (i = 0; i < height; i++) {
    row_pointers[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
    if (!row_pointers[i]) {
      printf("Memory allocation failed.\n");
      for (j = 0; j < i; j++)
        free(row_pointers[j]);
      free(row_pointers);
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(file);
      return;
    }
  }

  png_read_image(png_ptr, row_pointers);

  uint32_t redFreq[256] = {0};
  uint32_t greenFreq[256] = {0};
  uint32_t blueFreq[256] = {0};

  for (i = 0; i < height; i++) {
    png_bytep row = row_pointers[i];
    for (j = 0; j < width; j++) {
      png_bytep px = &(row[j * 3]);
      redFreq[px[0]]++;
      greenFreq[px[1]]++;
      blueFreq[px[2]]++;
    }
  }

  uint8_t dominantRed = 0, dominantGreen = 0, dominantBlue = 0;
  uint32_t maxRedFreq = 0, maxGreenFreq = 0, maxBlueFreq = 0;

  for (i = 0; i < 256; i++) {
    if (redFreq[i] > maxRedFreq) {
      maxRedFreq = redFreq[i];
      dominantRed = (uint8_t)i;
    }
    if (greenFreq[i] > maxGreenFreq) {
      maxGreenFreq = greenFreq[i];
      dominantGreen = (uint8_t)i;
    }
    if (blueFreq[i] > maxBlueFreq) {
      maxBlueFreq = blueFreq[i];
      dominantBlue = (uint8_t)i;
    }
  }

  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    printf("Could not create the output file.\n");
    for (i = 0; i < height; i++)
      free(row_pointers[i]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);
    return;
  }

  fprintf(outputFile, "Resolution: %dx%d\n", width, height);
  fprintf(outputFile, "Dominant color: R=%u, G=%u, B=%u\n", dominantRed, dominantGreen, dominantBlue);

  fclose(outputFile);

  for (i = 0; i < height; i++)
    free(row_pointers[i]);
  free(row_pointers);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(file);
}

int extractPNGStats_main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: ./png_stats <input_file>\n");
    return 1;
  }

  const char *filename = argv[1];
  const char *outputFilename = "png_stats.txt";
  extractPNGStats(filename, outputFilename);

  return 0;
}
