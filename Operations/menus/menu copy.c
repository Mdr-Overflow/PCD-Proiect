#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
// headers
#include "png_blurr.h" 
#include "png_grayscale.h"
#include "jpeg_blurr.h"
#include "png_tint.h"
#include "jpeg_grayscale.h"
#include "jpeg_tint.h"
#include "bmp_tint.h"



void conversionsMenu(const char* filename) {
    int choice;
    const char *inputFileName;
    const char *outputFileName;

    while (1) {
        printf("\nConversions Menu:\n");
        printf("1. bmp2jpeg\n");
        printf("2. bmp2png\n");
        printf("3. jpeg2bmp\n");
        printf("4. jpeg2png\n");
        printf("5. png2bmp\n");
        printf("6. png2jpeg\n");
        printf("7. Run all\n");
        printf("8. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("bmp2jpeg selected for file: %s\n", filename);
                  // Apply BMP to JPEG conversion
                inputFileName = filename; // Assuming argv[1] contains the input file name
                outputFileName = "tint_output.bmp";
                void apply_blueish_tint(const char *inputFileName, const char *outputFileName);

                apply_blueish_tint(inputFileName, outputFileName); 
                printf("BMP2JPEG blur applied successfully.\n");
                break;
            case 2:
                printf("bmp2png selected for file: %s\n", filename);
                break;
            case 3:
                printf("jpeg2bmp selected for file: %s\n", filename);
                break;
            case 4:
                printf("jpeg2png selected for file: %s\n", filename);
                break;
            case 5:
                printf("png2bmp selected for file: %s\n", filename);
                break;
            case 6:
                printf("png2jpeg selected for file: %s\n", filename);
                break;
            case 7:
                printf("Running all conversions for file: %s\n", filename);
                printf("bmp2jpeg\n");
                printf("bmp2png\n");
                printf("jpeg2bmp\n");
                printf("jpeg2png\n");
                printf("png2bmp\n");
                printf("png2jpeg\n");
                break;
            case 8:
                printf("Returning to main menu...\n");
                return;
            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }
}

void filtersMenu(const char* filename) {
    int choice;
    const char *inputFileName;
    const char *outputFileName;


    while (1) {
        printf("\nFilters Menu:\n");
        printf("1. bmptint\n");
        printf("2. jpegblurr\n");
        printf("3. jpeggrayscale\n");
        printf("4. jpegtint\n");
        printf("5. pngblurr\n");
        printf("6. pnggrayscale\n");
        printf("7. pngtint\n");
        printf("8. Run all\n");
        printf("9. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("bmptint selected for file: %s\n", filename);
                 // Apply bmp tint
                inputFileName = filename; 
                outputFileName = "tint_output.bmp";
                void apply_blueish_tint(const char *inputFileName, const char *outputFileName);

                apply_blueish_tint(inputFileName, outputFileName); 
                printf("JPEG blur applied successfully.\n");
                break;
            case 2:
                printf("jpegblurr selected for file: %s\n", filename);
                 // Apply JPEG blur
                inputFileName = filename; 
                outputFileName = "blurred_output.jpeg";
                void blurJPG(const char *inputFileName, const char *outputFileName);

                blurJPG(inputFileName, outputFileName); 
                printf("JPEG blur applied successfully.\n");
                break;
            case 3:
                printf("jpeggrayscale selected for file: %s\n", filename);
                // Apply JPEG grayscale
                // tinted or gray with vertical stripes!!!!!!!!!!!!!!!!!!!1
                inputFileName = filename; 
                outputFileName = "grayscale_output.jpeg";
                void JPGToGrayscale(const char *inputFileName, const char *outputFileName);

                JPGToGrayscale(inputFileName, outputFileName); 
                printf("JPEG grayscale applied successfully.\n");
                break;
            case 4:
                printf("jpegtint selected for file: %s\n", filename);
                // Apply JPEG tint
                inputFileName = filename; 
                outputFileName = "tint_output.jpeg";
                void blueish_tint_jpg(const char *inputFileName, const char *outputFileName);

                blueish_tint_jpg(inputFileName, outputFileName); 
                printf("JPEG tint applied successfully.\n");
                break;
            case 5:
                printf("pngblurr selected for file: %s\n", filename);
                // Apply PNG blur
                inputFileName = filename; 
                outputFileName = "blurred_output.png";

                blurPNG(inputFileName, outputFileName); 
                printf("PNG blur applied successfully.\n");
                break;
            case 6:
                printf("pnggrayscale selected for file: %s\n", filename);
                 // Apply PNG blur
                const char *inputFileName = filename; 
                const char *outputFileName = "grayscaled_output.png";
                void grayOutPNG(const char *inputFileName, const char *outputFileName);

                grayOutPNG(inputFileName, outputFileName); 
                printf("PNG grasycale applied successfully.\n");
                break;
            case 7:
                printf("pngtint selected for file: %s\n", filename);
                // Apply PNG tint
                inputFileName = filename; 
                outputFileName = "tint_output.png";
                void blueish_tint(const char *inputFileName, const char *outputFileName);

                blueish_tint(inputFileName, outputFileName); 
                printf("PNG tint applied successfully.\n");
                break;
            case 8:
                printf("Running all filters for file: %s\n", filename);
                printf("bmptint\n");
                printf("jpegblurr\n");
                printf("jpeggrayscale\n");
                printf("jpegtint\n");
                printf("pngblurr\n");
                printf("pnggrayscale\n");
                printf("pngtint\n");
                break;
            case 9:
                printf("Returning to main menu...\n");
                return;
            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }
}

int main(int argc, char** argv) {
    int choice;

    while (1) {
        printf("Main Menu:\n");
        printf("1. Conversions\n");
        printf("2. Filters\n");
        printf("3. Compression\n");
        printf("4. Stats\n");
        printf("5. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                conversionsMenu(argv[1]);
                break;
            case 2:
                filtersMenu(argv[1]);
                break;
            case 3:
                printf("Compression selected\n");
                break;
            case 4:
                printf("Stats selected\n");
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }

    return 0;
}