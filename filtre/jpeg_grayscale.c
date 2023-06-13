#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
//#include <vips/vips.h>

void convertToGrayscale(const char *inputFile, const char *outputFile) {
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

  // Allocate memory for the buffer
  rowStride = cinfo.output_width;
  buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * cinfo.output_height);
  for (int i = 0; i < cinfo.output_height; i++) {
    buffer[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * rowStride);
  }

  // Open the output file
  outputFileStream = fopen(outputFile, "wb");
  if (outputFileStream == NULL) {
    printf("Could not create output file.\n");
    jpeg_destroy_decompress(&cinfo);
    fclose(inputFileStream);
    return;
  }

  // Configure the output compression settings
  struct jpeg_compress_struct outcinfo;
  outcinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&outcinfo);
  jpeg_stdio_dest(&outcinfo, outputFileStream);

  outcinfo.image_width = cinfo.output_width;
  outcinfo.image_height = cinfo.output_height;
  outcinfo.input_components = 1;
  outcinfo.in_color_space = JCS_GRAYSCALE;

  jpeg_set_defaults(&outcinfo);
  jpeg_set_quality(&outcinfo, 100, TRUE);
  jpeg_start_compress(&outcinfo, TRUE);

  // Process the image row by row and convert to grayscale
  while (cinfo.output_scanline < cinfo.output_height) {
    int row = cinfo.output_scanline;
    jpeg_read_scanlines(&cinfo, &buffer[row], 1);
    jpeg_write_scanlines(&outcinfo, &buffer[row], 1);
  }

  // Finish writing the output file
  jpeg_finish_compress(&outcinfo);

  // Cleanup and release resources
  jpeg_destroy_compress(&outcinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(inputFileStream);
  fclose(outputFileStream);

  // Free allocated memory
  for (int i = 0; i < cinfo.output_height; i++) {
    free(buffer[i]);
  }
  free(buffer);
}



int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  if (vips_init(argv[0]) != 0) {
    vips_error_exit(NULL);
  }

  VipsImage *inputImage = vips_image_new_from_file(argv[1], NULL);
  if (inputImage == NULL) {
    vips_error_exit(NULL);
  }

  VipsImage *outputImage;
  if (vips_colourspace(inputImage, &outputImage, VIPS_INTERPRETATION_B_W, NULL) != 0) {
    vips_error_exit(NULL);
  }

  if (vips_image_write_to_file(outputImage, "output.jpg", NULL) != 0) {
    vips_error_exit(NULL);
  }

  g_object_unref(inputImage);
  g_object_unref(outputImage);
  vips_shutdown();

  return 0;
}
