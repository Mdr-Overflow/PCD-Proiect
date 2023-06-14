#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>


// command: gcc png2jpeg.c -o png2jpeg -lpng -ljpeg && ./png2jpeg dog.png
// Function to convert PNG to JPEG
int convertPNGtoJPEG(const char *inputFilename, const char *outputFilename,
                     int quality) {
  // Open the PNG input file
  FILE *inputFile = fopen(inputFilename, "rb");
  if (!inputFile) {
    fprintf(stderr, "Error opening input file: %s\n", inputFilename);
    return 0;
  }

  // Initialize the PNG read structure
  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fprintf(stderr, "Error initializing PNG read structure\n");
    fclose(inputFile);
    return 0;
  }

  // Initialize the PNG info structure
  png_infop info = png_create_info_struct(png);
  if (!info) {
    fprintf(stderr, "Error initializing PNG info structure\n");
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(inputFile);
    return 0;
  }

  // Set error handling
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "Error during PNG read\n");
    png_destroy_read_struct(&png, &info, NULL);
    fclose(inputFile);
    return 0;
  }

  // Set the input file
  png_init_io(png, inputFile);

  // Read the PNG header
  png_read_info(png, info);

  // Get the image dimensions
  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  int color_type = png_get_color_type(png, info);
  int bit_depth = png_get_bit_depth(png, info);

  // Update the color type to RGB if necessary
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  if (bit_depth == 16)
    png_set_strip_16(png);

  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  // Update the info structure
  png_read_update_info(png, info);

  // Allocate memory for the pixel buffer
  png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
  }

  // Read the PNG image data
  png_read_image(png, row_pointers);

  // Clean up the PNG structures
  png_destroy_read_struct(&png, &info, NULL);
  fclose(inputFile);

  // Open the JPEG output file
  FILE *outputFile = fopen(outputFilename, "wb");
  if (!outputFile) {
    fprintf(stderr, "Error opening output file: %s\n", outputFilename);
    return 0;
  }

  // Initialize the JPEG compression object
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // Set the output file
  jpeg_stdio_dest(&cinfo, outputFile);

  // Set image parameters
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB; // Use JCS_RGB for preserving colors

  // Set default compression parameters
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  // Start the JPEG compression
  jpeg_start_compress(&cinfo, TRUE);

  // Create a new buffer with proper alignment
  int row_stride = width * 3; // RGB has 3 bytes per pixel
  JSAMPARRAY row_buffer = (*cinfo.mem->alloc_sarray)(
      (j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  // Write the JPEG image data
  while (cinfo.next_scanline < cinfo.image_height) {
    // Copy row data from the PNG buffer to the new buffer
    for (int i = 0; i < width; i++) {
      row_buffer[0][3 * i] = row_pointers[cinfo.next_scanline][4 * i];
      row_buffer[0][3 * i + 1] = row_pointers[cinfo.next_scanline][4 * i + 1];
      row_buffer[0][3 * i + 2] = row_pointers[cinfo.next_scanline][4 * i + 2];
    }
    // Write the row to the JPEG file
    jpeg_write_scanlines(&cinfo, row_buffer, 1);
  }
  // Finish the JPEG compression
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(outputFile);

  // Free the pixel buffer memory
  for (int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  printf("Conversion completed successfully.\n");

  return 1;
}

int convertPNGtoJPEG_main(int argc, char **argv) {
  const char *inputFilename = argv[1];
  const char *outputFilename = "output.jpeg";
  int quality = 100; // JPEG compression quality (0-100)

  if (convertPNGtoJPEG(inputFilename, outputFilename, quality)) {
    printf("PNG to JPEG conversion successful!\n");
  } else {
    printf("PNG to JPEG conversion failed.\n");
  }

  return 0;
}
