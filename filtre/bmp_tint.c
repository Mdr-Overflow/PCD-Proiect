// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>

// #pragma pack(push, 1)
// typedef struct {
//   uint16_t signature;
//   uint32_t file_size;
//   uint32_t reserved;
//   uint32_t data_offset;
// } BMPHeader;

// typedef struct {
//   uint32_t header_size;
//   int32_t width;
//   int32_t height;
//   uint16_t planes;
//   uint16_t bit_depth;
//   uint32_t compression;
//   uint32_t image_size;
//   int32_t x_pixels_per_meter;
//   int32_t y_pixels_per_meter;
//   uint32_t colors_used;
//   uint32_t colors_important;
//   uint64_t red_mask;
//   uint64_t green_mask;
//   uint64_t blue_mask;
//   uint64_t alpha_mask;
//   uint32_t color_space_type;
//   uint8_t unused[36];
// } BMPInfoHeader;
// #pragma pack(pop)

// void apply_blueish_tint(const char *input_file, const char *output_file) {
//   FILE *input = fopen(input_file, "rb");
//   if (!input) {
//     printf("Error opening input file.\n");
//     return;
//   }

//   // Read BMP header
//   BMPHeader header;
//   fread(&header, sizeof(BMPHeader), 1, input);

//   // Verify BMP signature
//   if (header.signature != 0x4D42) {
//     printf("Unsupported BMP format.\n");
//     fclose(input);
//     return;
//   }

//   // Read BMP info header
//   BMPInfoHeader info_header;
//   fread(&info_header, sizeof(BMPInfoHeader), 1, input);

//   // Verify color depth
//   if (info_header.bit_depth != 24 && info_header.bit_depth != 32 &&
//       info_header.bit_depth != 64) {
//     printf("Unsupported BMP format. Only 24-bit, 32-bit, and 64-bit BMP
//     images "
//            "are supported.\n");
//     fclose(input);
//     return;
//   }

//   // Calculate row size
//   uint32_t row_size =
//       ((info_header.bit_depth * info_header.width + 31) / 32) * 4;

//   // Calculate image data size
//   uint32_t data_size = row_size * abs(info_header.height);

//   // Allocate memory for image data
//   uint8_t *data = (uint8_t *)malloc(data_size);
//   if (!data) {
//     printf("Memory allocation failed.\n");
//     fclose(input);
//     return;
//   }

//   // Read image data
//   fread(data, data_size, 1, input);

//   // Apply blueish tint to the image data
//   uint32_t red_mask = (uint32_t)info_header.red_mask;
//   uint32_t green_mask = (uint32_t)info_header.green_mask;
//   uint32_t blue_mask = (uint32_t)info_header.blue_mask;
//   uint32_t alpha_mask = (uint32_t)info_header.alpha_mask;

//   for (uint32_t i = 0; i < data_size; i += (info_header.bit_depth / 8)) {
//     uint32_t *pixel = (uint32_t *)(data + i);

//     uint32_t red = ((*pixel) & red_mask) >> __builtin_ctz(red_mask);
//     uint32_t green = ((*pixel) & green_mask) >> __builtin_ctz(green_mask);
//     uint32_t blue = ((*pixel) & blue_mask) >> __builtin_ctz(blue_mask);
//     uint32_t alpha = ((*pixel) & alpha_mask) >> __builtin_ctz(alpha_mask);

//     // Apply the blueish tint by increasing the blue channel
//     blue = (blue > 100) ? blue : 100; // Increase the blue channel (adjust
//     the
//                                       // threshold as desired)

//     // Update the pixel value
//     *pixel = (alpha << __builtin_ctz(alpha_mask)) |
//              (blue << __builtin_ctz(blue_mask)) |
//              (green << __builtin_ctz(green_mask)) |
//              (red << __builtin_ctz(red_mask));
//   }

//   // Create the output file
//   FILE *output = fopen(output_file, "wb");
//   if (!output) {
//     printf("Error creating output file.\n");
//     free(data);
//     fclose(input);
//     return;
//   }

//   // Write the BMP header
//   fwrite(&header, sizeof(BMPHeader), 1, output);

//   // Write the BMP info header
//   fwrite(&info_header, sizeof(BMPInfoHeader), 1, output);

//   // Write the image data
//   fwrite(data, data_size, 1, output);

//   // Close the files
//   fclose(input);
//   fclose(output);

//   // Free the memory
//   free(data);

//   printf("Image processing complete.\n");
// }

// int main(int argc, char **argv) {
//   if (argc < 2) {
//     printf("Usage: ./bmp_tint input_file.bmp\n");
//     return 0;
//   }
//   const char *input_file = argv[1];
//   const char *output_file = "bmp_tint_output.bmp";

//   apply_blueish_tint(input_file, output_file);

//   return 0;
// }

///////////////////////////////////////////////////////////////////update the
/// blue artifacts
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)

typedef struct {
  uint16_t signature;
  uint32_t file_size;
  uint32_t reserved;
  uint32_t data_offset;
} BMPHeader;

typedef struct {
  uint32_t header_size;
  int32_t width;
  int32_t height;
  uint16_t planes;
  uint16_t bit_depth;
  uint32_t compression;
  uint32_t image_size;
  int32_t x_pixels_per_meter;
  int32_t y_pixels_per_meter;
  uint32_t colors_used;
  uint32_t colors_important;
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
  uint32_t alpha_mask;
  uint32_t color_space_type;
  uint8_t unused[36];
} BMPInfoHeader;

#pragma pack(pop)

void apply_blueish_tint(const char *input_file, const char *output_file) {
  FILE *input = fopen(input_file, "rb");
  if (!input) {
    printf("Error opening input file.\n");
    return;
  }

  // Read BMP header
  BMPHeader header;
  fread(&header, sizeof(BMPHeader), 1, input);

  // Verify BMP signature
  if (header.signature != 0x4D42) {
    printf("Unsupported BMP format.\n");
    fclose(input);
    return;
  }

  // Read BMP info header
  BMPInfoHeader info_header;
  fread(&info_header, sizeof(BMPInfoHeader), 1, input);

  // Verify color depth
  if (info_header.bit_depth != 24 && info_header.bit_depth != 32 &&
      info_header.bit_depth != 48 && info_header.bit_depth != 64) {
    printf("Unsupported BMP format. Only 24-bit, 32-bit, 48-bit, and 64-bit "
           "BMP images are supported.\n");
    fclose(input);
    return;
  }

  // Calculate row size
  uint32_t row_size =
      ((info_header.bit_depth * info_header.width + 31) / 32) * 4;

  // Calculate image data size
  uint32_t data_size = row_size * abs(info_header.height);

  // Allocate memory for image data
  uint8_t *data = (uint8_t *)malloc(data_size);
  if (!data) {
    printf("Memory allocation failed.\n");
    fclose(input);
    return;
  }

  // Read image data
  fread(data, data_size, 1, input);

  // Apply blueish tint to the image data
  uint32_t red_mask = info_header.red_mask;
  uint32_t green_mask = info_header.green_mask;
  uint32_t blue_mask = info_header.blue_mask;
  uint32_t alpha_mask = info_header.alpha_mask;

  // Apply blueish tint to the image data
  for (uint32_t i = 0; i < data_size; i += (info_header.bit_depth / 8)) {
    uint8_t *pixel = data + i;

    uint8_t blue = pixel[0];
    uint8_t green = pixel[1];
    uint8_t red = pixel[2];

    // Increase the blue channel to achieve a blueish tint
    blue = (blue > 100) ? blue : 100; // Adjust the threshold as desired

    // Decrease the red and green channels to reduce their intensity
    red = (red > 50) ? red - 50
                     : 0; // Adjust the threshold and decrement as desired
    green = (green > 50) ? green + 50
                         : 0; // Adjust the threshold and decrement as desired

    // Update the pixel value
    pixel[0] = blue;
    pixel[1] = green;
    pixel[2] = red;
  }

  // Create the output file
  FILE *output = fopen(output_file, "wb");
  if (!output) {
    printf("Error creating output file.\n");
    free(data);
    fclose(input);
    return;
  }

  // Write the BMP header
  fwrite(&header, sizeof(BMPHeader), 1, output);

  // Write the BMP info header
  fwrite(&info_header, sizeof(BMPInfoHeader), 1, output);

  // Write the image data
  fwrite(data, data_size, 1, output);

  // Close the files
  fclose(input);
  fclose(output);

  // Free the memory
  free(data);

  printf("Image processing complete.\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: ./bmp_tint input_file.bmp\n");
    return 0;
  }
  const char *input_file = argv[1];
  const char *output_file = "bmp_tint_output.bmp";

  apply_blueish_tint(input_file, output_file);

  return 0;
}
