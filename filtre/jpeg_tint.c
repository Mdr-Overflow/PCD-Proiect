#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void blueish_tint(const char* input_file, const char* output_file) {
    // Open the input JPEG file
    FILE* file = fopen(input_file, "rb");
    if (!file) {
        printf("Error opening input file.\n");
        return;
    }

    // Initialize JPEG structures
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Set input file
    jpeg_stdio_src(&cinfo, file);

    // Read JPEG header
    jpeg_read_header(&cinfo, TRUE);

    // Start decompression
    jpeg_start_decompress(&cinfo);

    // Get image information
    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int channels = cinfo.num_components;

    // Allocate memory for image buffer
    JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    for (int i = 0; i < height; ++i) {
        buffer[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * width * channels);
    }

    // Read scanlines and store in buffer
    while (cinfo.output_scanline < cinfo.output_height) {
        int row = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &buffer[row], 1);
    }

    // Modify the image data to apply the blueish tint
    for (int y = 0; y < height; ++y) {
        JSAMPROW row = buffer[y];
        for (int x = 0; x < width; ++x) {
            JSAMPLE* pixel = &(row[x * channels]);

            // Apply the blueish tint by increasing the blue channel
            pixel[2] = (pixel[2] > 100) ? pixel[2] : 100;  // Increase the blue channel (adjust the threshold as desired)
        }
    }

    // Create output JPEG file
    FILE* output = fopen(output_file, "wb");
    if (!output) {
        printf("Error creating output file.\n");
        return;
    }

    // Initialize JPEG structures for output
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr_out;
    cinfo_out.err = jpeg_std_error(&jerr_out);
    jpeg_create_compress(&cinfo_out);

    // Set output file
    jpeg_stdio_dest(&cinfo_out, output);

    // Set output image parameters
    cinfo_out.image_width = width;
    cinfo_out.image_height = height;
    cinfo_out.input_components = channels;
    cinfo_out.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo_out);

    // Start compression
    jpeg_start_compress(&cinfo_out, TRUE);

    // Write scanlines from buffer to output
    while (cinfo_out.next_scanline < cinfo_out.image_height) {
        int row = cinfo_out.next_scanline;
        jpeg_write_scanlines(&cinfo_out, &buffer[row], 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo_out);

    // Clean up resources
    for (int i = 0; i < height; ++i) {
        free(buffer[i]);
    }
    free(buffer);

    // Destroy JPEG structures
    jpeg_destroy_compress(&cinfo_out);
    jpeg_destroy_decompress(&cinfo);

    // Close input and output files
    fclose(file);
    fclose(output);

    printf("Image processing complete.\n");
}

int main(int argc, char ** argv) {
    const char* input_file = argv[1];
    const char* output_file = "jpeg_tint_output.jpg";
    blueish_tint(input_file, output_file);

    return 0;
}
