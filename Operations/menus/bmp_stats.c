#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint16_t signature;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} BMPHeader;

typedef struct {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t totalColors;
    uint32_t importantColors;
} BMPInfoHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

void extractBMPStats(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open the BMP file.\n");
        return;
    }

    BMPHeader bmpHeader;
    BMPInfoHeader bmpInfoHeader;

    // Read BMP header
    fread(&bmpHeader, sizeof(BMPHeader), 1, file);
    if (bmpHeader.signature != 0x4D42) {
        printf("Invalid BMP file.\n");
        fclose(file);
        return;
    }

    // Read BMP info header
    fread(&bmpInfoHeader, sizeof(BMPInfoHeader), 1, file);
    if (bmpInfoHeader.headerSize != 40) {
        printf("Invalid BMP file.\n");
        fclose(file);
        return;
    }

    // Extract resolution
    int32_t width = bmpInfoHeader.width;
    int32_t height = bmpInfoHeader.height;

    // Calculate image size in bytes
    uint32_t imageSize = bmpInfoHeader.imageSize;
    if (imageSize == 0)
        imageSize = bmpHeader.fileSize - bmpHeader.dataOffset;

    // Calculate number of pixels
    uint32_t numPixels = imageSize / sizeof(Pixel);

    // Allocate memory for pixel data
    Pixel* pixels = (Pixel*)malloc(imageSize);
    if (!pixels) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return;
    }

    // Read pixel data
    fseek(file, bmpHeader.dataOffset, SEEK_SET);
    fread(pixels, sizeof(Pixel), numPixels, file);

    fclose(file);

    // Calculate dominant color
    uint32_t i;
    uint32_t redFreq[256] = { 0 };
    uint32_t greenFreq[256] = { 0 };
    uint32_t blueFreq[256] = { 0 };

    for (i = 0; i < numPixels; i++) {
        Pixel pixel = pixels[i];
        redFreq[pixel.red]++;
        greenFreq[pixel.green]++;
        blueFreq[pixel.blue]++;
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

    // Hardcoded output file name
    const char* outputFilename = "bmp_stats.txt";

    // Write stats to output file
    FILE* outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        printf("Could not open the output file.\n");
        free(pixels);
        return;
    }

    fprintf(outputFile, "Resolution: %dx%d\n", width, height);
    fprintf(outputFile, "Dominant color: R=%u, G=%u, B=%u\n", dominantRed, dominantGreen, dominantBlue);

    fclose(outputFile);

    free(pixels);
}

int extractBMPStats_main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./bmp_stats <input_file>\n");
        return 1;
    }

    const char* filename = argv[1];
    extractBMPStats(filename);

    return 0;
}
