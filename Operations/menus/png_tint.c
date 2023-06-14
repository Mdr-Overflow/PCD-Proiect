#include <png.h>
#include <stdio.h>
#include <stdlib.h>

void blueish_tint(const char *input_file, const char *output_file) {
  // Open the input PNG file
  FILE *file = fopen(input_file, "rb");
  if (!file) {
    printf("Error opening input file.\n");
    return;
  }

  // Read the PNG header
  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fclose(file);
    printf("Error creating PNG read struct.\n");
    return;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(file);
    printf("Error creating PNG info struct.\n");
    return;
  }

  if (setjmp(png_jmpbuf(png))) {
    png_destroy_read_struct(&png, &info, NULL);
    fclose(file);
    printf("Error reading PNG file.\n");
    return;
  }

  png_init_io(png, file);
  png_read_info(png, info);

  // Get image information
  png_uint_32 width = png_get_image_width(png, info);
  png_uint_32 height = png_get_image_height(png, info);
  int color_type = png_get_color_type(png, info);
  int bit_depth = png_get_bit_depth(png, info);

  // Ensure the input image is in RGBA format
  if (color_type != PNG_COLOR_TYPE_RGBA) {
    fclose(file);
    printf("Input PNG image is not in RGBA format.\n");
    return;
  }

  // Allocate memory for image data
  png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (png_uint_32 y = 0; y < height; ++y) {
    row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
  }

  // Read image data
  png_read_image(png, row_pointers);

  // Modify the image data to apply the blueish tint
  for (png_uint_32 y = 0; y < height; ++y) {
    png_bytep row = row_pointers[y];
    for (png_uint_32 x = 0; x < width; ++x) {
      png_bytep px = &(row[x * 4]); // Each pixel has 4 bytes (RGBA)

      // Apply the blueish tint by increasing the blue channel
      px[2] = (px[2] > 100) ? px[2] : 100; // Increase the blue channel (adjust
                                           // the threshold as desired)
    }
  }

  // Create output PNG file
  FILE *output = fopen(output_file, "wb");
  if (!output) {
    printf("Error creating output file.\n");
    return;
  }

  // Write the modified image data to the output file
  png_structp png_out =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_out) {
    fclose(output);
    printf("Error creating PNG write struct.\n");
    return;
  }

  png_infop info_out = png_create_info_struct(png_out);
  if (!info_out) {
    png_destroy_write_struct(&png_out, NULL);
    fclose(output);
    printf("Error creating PNG info struct.\n");
    return;
  }

  png_init_io(png_out, output);
  png_set_IHDR(png_out, info_out, width, height, bit_depth, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_out, info_out);
  png_write_image(png_out, row_pointers);
  png_write_end(png_out, NULL);

  // Clean up resources
  for (png_uint_32 y = 0; y < height; ++y) {
    free(row_pointers[y]);
  }
  free(row_pointers);
  png_destroy_read_struct(&png, &info, NULL);
  png_destroy_write_struct(&png_out, &info_out);
  fclose(file);
  fclose(output);

  printf("Image processing complete.\n");
}

int blueish_tint_main(int argc, char **argv) {
  if (argc < 2) {
    printf("Please provide the input file path.\n");
    return 1;
  }

  const char *input_file = argv[1];
  const char *output_file = "png_tint_output.png";
  blueish_tint(input_file, output_file);

  return 0;
}