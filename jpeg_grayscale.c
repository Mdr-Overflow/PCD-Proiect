#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void JPGToGrayscale(const char *inputFile, const char *outputFile) {
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
  rowStride = cinfo.output_width * cinfo.output_components;
  rowStride = (rowStride + 3) & ~3; // Align row stride to multiple of 4
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
  struct jpeg_error_mgr outjerr;
  outcinfo.err = jpeg_std_error(&outjerr);
  jpeg_create_compress(&outcinfo);
  jpeg_stdio_dest(&outcinfo, outputFileStream);

  outcinfo.image_width = cinfo.output_width;
  outcinfo.image_height = cinfo.output_height;
  outcinfo.input_components = cinfo.output_components;
  outcinfo.in_color_space = cinfo.out_color_space;
  jpeg_copy_critical_parameters(&cinfo, &outcinfo);

  // Set color conversion parameters
  outcinfo.comp_info = (jpeg_component_info *)malloc(sizeof(jpeg_component_info) * outcinfo.num_components);
  for (int i = 0; i < outcinfo.num_components; i++) {
    outcinfo.comp_info[i] = cinfo.comp_info[i];
    outcinfo.comp_info[i].h_samp_factor = 1;
    outcinfo.comp_info[i].v_samp_factor = 1;
  }

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

  // Free the buffer memory
  for (int i = 0; i < cinfo.output_height; i++) {
    free(buffer[i]);
  }
  free(buffer);
}

int JPGToGrayscale_main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  const char *inputFile = argv[1];
  const char *outputFile = "output.jpg"; // Output file name can be modified if desired

  JPGToGrayscale(inputFile, outputFile);

  return 0;
}
