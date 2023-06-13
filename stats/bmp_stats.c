#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;

typedef struct {
    uint32_t width;
    uint32_t height;
    Pixel* data;
} Image;

Image* readImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open the image file.\n");
        return NULL;
    }

    // Read image dimensions
    uint32_t width, height;
    fseek(file, 18, SEEK_SET);
    fread(&width, sizeof(uint32_t), 1, file);
    fread(&height, sizeof(uint32_t), 1, file);

    // Allocate memory for image data
    Image* image = (Image*)malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->data = (Pixel*)malloc(width * height * sizeof(Pixel));

    // Read pixel data
    fseek(file, 54, SEEK_SET);
    fread(image->data, sizeof(Pixel), width * height, file);

    fclose(file);

    return image;
}

void freeImage(Image* image) {
    free(image->data);
    free(image);
}

void calculateImageStats(Image* image) {
    uint32_t numPixels = image->width * image->height;
    uint32_t i;
    uint32_t sumRed = 0, sumGreen = 0, sumBlue = 0;
    uint8_t minRed = 255, minGreen = 255, minBlue = 255;
    uint8_t maxRed = 0, maxGreen = 0, maxBlue = 0;

    for (i = 0; i < numPixels; i++) {
        Pixel pixel = image->data[i];

        sumRed += pixel.red;
        sumGreen += pixel.green;
        sumBlue += pixel.blue;

        if (pixel.red < minRed)
            minRed = pixel.red;
        if (pixel.red > maxRed)
            maxRed = pixel.red;

        if (pixel.green < minGreen)
            minGreen = pixel.green;
        if (pixel.green > maxGreen)
            maxGreen = pixel.green;

        if (pixel.blue < minBlue)
            minBlue = pixel.blue;
        if (pixel.blue > maxBlue)
            maxBlue = pixel.blue;
    }

    double meanRed = (double)sumRed / numPixels;
    double meanGreen = (double)sumGreen / numPixels;
    double meanBlue = (double)sumBlue / numPixels;

    printf("Minimum pixel values: R=%u, G=%u, B=%u\n", minRed, minGreen, minBlue);
    printf("Maximum pixel values: R=%u, G=%u, B=%u\n", maxRed, maxGreen, maxBlue);
    printf("Mean pixel values: R=%.2f, G=%.2f, B=%.2f\n", meanRed, meanGreen, meanBlue);
}

void printImageResolution(Image* image) {
    printf("Image Resolution: %u x %u\n", image->width, image->height);
}

void calculateDominantColor(Image* image) {
    uint32_t numPixels = image->width * image->height;
    uint32_t i;
    uint32_t redFreq[256] = { 0 };
    uint32_t greenFreq[256] = { 0 };
    uint32_t blueFreq[256] = { 0 };

    for (i = 0; i < numPixels; i++) {
        Pixel pixel = image->data[i];
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

    printf("Dominant color: R=%u, G=%u, B=%u\n", dominantRed, dominantGreen, dominantBlue);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./image_stats <image_path>\n");
        return 1;
    }

    const char* filename = argv[1];
    Image* image = readImage(filename);

    if (!image)
        return 1;

    calculateImageStats(image);
    printImageResolution(image);
    calculateDominantColor(image);

    freeImage(image);

    return 0;
}

