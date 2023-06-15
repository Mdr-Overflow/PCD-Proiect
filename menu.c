#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
// filter headers
#include "png_blurr.h" 
#include "png_grayscale.h"
#include "jpeg_blurr.h"
#include "png_tint.h"
#include "jpeg_grayscale.h"
#include "jpeg_tint.h"
#include "bmp_tint.h"
// conversion header
#include "bmp2jpeg.h"
#include "bmp2png.h"
#include "jpeg2bmp.h"
#include "jpeg2png.h"
#include "png2bmp.h"
#include "png2jpeg.h"
// stats headers
#include "bmp_stats.h"
#include "jpeg_stats.h"
#include "png_stats.h"





//gcc Operations/menu.c Operations/bmp2jpeg.c Operations/bmp2png.c Operations/jpeg2bmp.c Operations/jpeg2png.c Operations/png2bmp.c Operations/png2jpeg.c Operations/bmp_tint.c Operations/jpeg_blurr.c Operations/jpeg_grayscale.c Operations/jpeg_tint.c Operations/png_blurr.c Operations/png_grayscale.c Operations/png_tint.c Operations/bmp_stats.c Operations/png_stats.c Operations/jpeg_stats.c -o menu -ljpeg -lpng && ./menu Operations/dog.png



// CONVERSIONS

//1
void* applyBMP2JPEG (void *filename) {
    printf("bmp2jpeg selected for file: %s\n", (char*)filename);
    // Apply BMP to JPEG conversion
    //  NEEDS 180 ROTATION, colors are off, overflow added on the other side
    const char * inputFileName = filename;
    const char *outputFileName = "BMP2JPEG_output.jpg";
    void convertBMPtoJPEG(const char *inputFileName, const char *outputFileName);
    convertBMPtoJPEG(inputFileName, outputFileName); 
    printf("bmp2jpeg conversion applied successfully.\n");
    pthread_exit(NULL);
}

//2
void* applyBMP2PNG (void *filename) {
    printf("bmp2png selected for file: %s\n", (char*)filename);
    // Apply BMP to PNG conversion
    // colors are off, overflow added on the other side
    const char *inputFileName = filename;
    const char *outputFileName = "BMP2PNG_output.jpg";
    void convertBMPtoPNG(const char *inputFileName, const char *outputFileName);
    convertBMPtoPNG(inputFileName, outputFileName); 
    printf("BMP2PNG conversion applied successfully.\n");
    pthread_exit(NULL);
}

//3
void* applyJPEG2BMP (void *filename) {
   printf("jpeg2bmp selected for file: %s\n", (char*)filename);
   // Apply JPEG to BMP conversion NICE
    const char *inputFileName = filename;
   const char *outputFileName = "JPEG2BMP_output.jpg";
   void convertJPEGtoBMP(const char *inputFileName, const char *outputFileName);
    convertJPEGtoBMP(inputFileName, outputFileName); 
     printf("JPEG2BMP conversion applied successfully.\n");
     pthread_exit(NULL);
}

//4
void* applyJPEG2PNG (void *filename) {
    printf("jpeg2png selected for file: %s\n",(char*)filename);
    // Apply JPEG to PNG conversion NICE
    const char *inputFileName = filename;
    const char *outputFileName = "JPEG2PNG_output.jpg";
    void convertJPEGtoPNG(const char *inputFileName, const char *outputFileName);
    convertJPEGtoPNG(inputFileName, outputFileName); 
    printf("JPEG2PNG conversion applied successfully.\n");
    pthread_exit(NULL);
}

//5
void* applyPNG2BMP (void *filename) {
    printf("png2bmp selected for file: %s\n", (char*)filename);
    // Apply PNG to BMP conversion NICE
    const char *inputFileName = filename;
    const char *outputFileName = "PNG2BMP_output.jpg";
    void convertPNGtoBMP(const char *inputFileName, const char *outputFileName);
    convertPNGtoBMP(inputFileName, outputFileName); 
    printf("PNG2BMP conversion applied successfully.\n");
    pthread_exit(NULL);
}

//6
void* applyPNG2JPEG (void *filename) {
   printf("png2jpeg selected for file: %s\n", (char*)filename);
   // Apply PNG to JPEG conversion NICE
   const char *inputFileName = filename;
   const char *outputFileName = "PNG2JPEG_output.jpg";
   void convertPNGtoJPEG(const char *inputFileName, const char *outputFileName);
   convertPNGtoJPEG(inputFileName, outputFileName); 
   printf("PNG2JPEG conversion applied successfully.\n");
   pthread_exit(NULL);
}



void conversionsMenu(const char* filename) {
    int choice;
    pthread_t thread;
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
                pthread_create(&thread, NULL, applyBMP2JPEG, (void*)filename);
                pthread_join(thread, NULL);
                //applyBMP2JPEG(filename);
                break;
            case 2:
                pthread_create(&thread, NULL, applyBMP2PNG, (void*)filename);
                pthread_join(thread, NULL);
               //applyBMP2PNG(filename);
                break;
            case 3:
                pthread_create(&thread, NULL, applyJPEG2BMP, (void*)filename);
                pthread_join(thread, NULL);
               //applyJPEG2BMP(filename);
                break;
            case 4:
                pthread_create(&thread, NULL, applyJPEG2PNG, (void*)filename);
                pthread_join(thread, NULL);
               //applyJPEG2PNG(filename);
                break;
            case 5:
                pthread_create(&thread, NULL, applyPNG2BMP, (void*)filename);
                pthread_join(thread, NULL);
               //applyPNG2BMP(filename);
                break;
            case 6:
                pthread_create(&thread, NULL, applyPNG2JPEG, (void*)filename);
                pthread_join(thread, NULL);
               //applyPNG2JPEG(filename);
                break;
            case 7:
                printf("Running all conversions for file: %s\n", filename);
                const char *fileExt = strrchr(filename, '.');  // Find the last occurrence of '.' in the filename
                if (fileExt != NULL) {
                      fileExt++;  // Move past the dot character
                }
                if(strcmp(fileExt, "bmp") == 0) {
                   pthread_create(&thread, NULL, applyBMP2JPEG, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyBMP2PNG, (void*)filename);
                   pthread_join(thread, NULL);
                }
                if(strcmp(fileExt, "png") == 0) {
                   pthread_create(&thread, NULL, applyPNG2BMP, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyPNG2JPEG, (void*)filename);
                   pthread_join(thread, NULL);
                }
                if(strcmp(fileExt, "jpg") == 0 || strcmp(fileExt, "jpeg") == 0) {
                   pthread_create(&thread, NULL, applyJPEG2BMP, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyJPEG2PNG, (void*)filename);
                   pthread_join(thread, NULL);
                }
                //pthread_exit(NULL);
                // pthread_create(&thread, NULL, applyRunAllConversions, (void*)filename);
                // pthread_join(thread, NULL);
                //applyRunAllConversions(filename);
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



// FILTERS

//1
void *applyBMPTint (void *filename) {
   printf("bmptint selected for file: %s\n", (char*)filename);
  // Apply bmp tint
   const char *inputFileName = filename; 
   const char *outputFileName = "tint_output.bmp";
   void apply_blueish_tint(const char *inputFileName, const char *outputFileName);
   apply_blueish_tint(inputFileName, outputFileName); 
   printf("JPEG blur applied successfully.\n");
   pthread_exit(NULL);
}

//2
void *applyJPEGBlurr (void *filename) {
   printf("jpegblurr selected for file: %s\n", (char*)filename);
  // Apply JPEG blur
  const char *inputFileName = filename; 
  const char *outputFileName = "blurred_output.jpeg";
   void blurJPG(const char *inputFileName, const char *outputFileName);
   blurJPG(inputFileName, outputFileName); 
   printf("JPEG blur applied successfully.\n");
   pthread_exit(NULL);
}

//3
void *applyJPEGGrayscale (void *filename) {
    printf("jpeggrayscale selected for file: %s\n", (char*)filename);
    // Apply JPEG grayscale
    // tinted or gray with vertical stripes!!!!!!!!!!!!!!!!!!!1
    const char *inputFileName = filename; 
    const char *outputFileName = "grayscale_output.jpeg";
    void JPGToGrayscale(const char *inputFileName, const char *outputFileName);
    JPGToGrayscale(inputFileName, outputFileName); 
    printf("JPEG grayscale applied successfully.\n");
    pthread_exit(NULL);
}

//4
void *applyJPEGTint (void *filename) {
    printf("jpegtint selected for file: %s\n", (char*)filename);
    // Apply JPEG tint
    const char *inputFileName = filename; 
    const char *outputFileName = "tint_output.jpeg";
    void blueish_tint_jpg(const char *inputFileName, const char *outputFileName);
    blueish_tint_jpg(inputFileName, outputFileName); 
    printf("JPEG tint applied successfully.\n");
    pthread_exit(NULL);
}

//5
void *applyPNGBlurr (void * filename) {
    printf("pngblurr selected for file: %s\n", (char*)filename);
    // Apply PNG blur
    const char *inputFileName = filename; 
    const char *outputFileName = "blurred_output.png";
    blurPNG(inputFileName, outputFileName); 
    printf("PNG blur applied successfully.\n");
    pthread_exit(NULL);
}

//6
void *applyPNGGrayscale (void * filename) {
   printf("pnggrayscale selected for file: %s\n", (char*)filename);
   // Apply PNG blur
   const char *inputFileName = filename; 
   const char *outputFileName = "grayscaled_output.png";
   void grayOutPNG(const char *inputFileName, const char *outputFileName);
   grayOutPNG(inputFileName, outputFileName); 
   printf("PNG grasycale applied successfully.\n");
   pthread_exit(NULL);
}

//7
void *applyPNGTint (void *filename) {
   printf("pngtint selected for file: %s\n", (char*)filename);
   // Apply PNG tint
   const char *inputFileName = filename; 
   const char *outputFileName = "tint_output.png";
   void blueish_tint(const char *inputFileName, const char *outputFileName);
   blueish_tint(inputFileName, outputFileName); 
   printf("PNG tint applied successfully.\n");
   pthread_exit(NULL);
}



void filtersMenu(const char* filename) {
    int choice;
    pthread_t thread;
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
                pthread_create(&thread, NULL, applyBMPTint, (void*)filename);
                pthread_join(thread, NULL);
               //applyBMPTint(filename);
                break;
            case 2:
                pthread_create(&thread, NULL, applyJPEGBlurr, (void*)filename);
                pthread_join(thread, NULL);
              //  applyJPEGBlurr(filename);
                break;
            case 3:
                 pthread_create(&thread, NULL, applyJPEGGrayscale, (void*)filename);
                pthread_join(thread, NULL);
              // applyJPEGGrayscale(filename);
                break;
            case 4:
                pthread_create(&thread, NULL, applyJPEGTint, (void*)filename);
                pthread_join(thread, NULL);
              //  applyJPEGTint(filename);
                break;
            case 5:
                 pthread_create(&thread, NULL, applyPNGBlurr, (void*)filename);
                pthread_join(thread, NULL);
              //  applyPNGBlurr(filename);
                break;
            case 6:
                pthread_create(&thread, NULL, applyPNGGrayscale, (void*)filename);
                pthread_join(thread, NULL);
              //  applyPNGGrayscale(filename);
                break;
            case 7:
                pthread_create(&thread, NULL, applyPNGTint, (void*)filename);
                pthread_join(thread, NULL);
                // applyPNGTint(filename);
                break;
            case 8:
                printf("Running all filters for file: %s\n", filename);
                 const char *fileExt = strrchr(filename, '.');  // Find the last occurrence of '.' in the filename
                if (fileExt != NULL) {
                      fileExt++;  // Move past the dot character
                }
                if(strcmp(fileExt, "bmp") == 0) {
                   pthread_create(&thread, NULL, applyBMPTint, (void*)filename);
                   pthread_join(thread, NULL);
                }
                if(strcmp(fileExt, "png") == 0) {
                  pthread_create(&thread, NULL, applyPNGBlurr, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyPNGGrayscale, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyPNGTint, (void*)filename);
                   pthread_join(thread, NULL);
                }
                if(strcmp(fileExt, "jpg") == 0 || strcmp(fileExt, "jpeg") == 0) {
                   pthread_create(&thread, NULL, applyJPEGBlurr, (void*)filename);
                   pthread_join(thread, NULL);
                  pthread_create(&thread, NULL, applyJPEGGrayscale, (void*)filename);
                   pthread_join(thread, NULL);
                   pthread_create(&thread, NULL, applyJPEGTint, (void*)filename);
                   pthread_join(thread, NULL);
                }
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


// STATS

void *BMPStats (void * filename) {
   printf("BMP stats selected for file: %s\n", (char*)filename);
   // Apply BMP stats
   const char *inputFileName = filename; 
   const char *outputFileName = "BMP_stats.txt";
   void extractBMPStats(const char *inputFileName);
   extractBMPStats(inputFileName); 
   printf("BMP stats applied successfully.\n");
   pthread_exit(NULL);
}

void *PNGStats (void * filename) {
   printf("PNG stats selected for file: %s\n", (char*)filename);
   // Apply PNG stats
   const char *inputFileName = filename; 
   const char *outputFileName = "PNG_stats.txt";
   void extractPNGStats(const char *filename, const char *outputFilename);
   extractPNGStats(inputFileName, outputFileName); 
   printf("PNG stats applied successfully.\n");
  pthread_exit(NULL);
}

void *JPGStats (void * filename) {
   printf("JPG stats selected for file: %s\n", (char*)filename);
   // Apply JPG stats
   const char *inputFileName = filename; 
   const char *outputFileName = "JPEG_stats.txt";
   void WriteJpegStatsToFile(const char *inputFileName, const char *outputFileName);
   WriteJpegStatsToFile(inputFileName, outputFileName); 
   printf("JPG stats applied successfully.\n");
   pthread_exit(NULL);
}

void statsMenu(const char* filename) {
    int choice;
    pthread_t thread;

    while (1) {
        printf("\nStats Menu:\n");
        printf("1. Calculate BMP stats\n");
        printf("2. Calculate PNG stats\n");
        printf("3. Calculate JPG stats\n");
        printf("4. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
            pthread_create(&thread, NULL, BMPStats, (void*)filename);
                pthread_join(thread, NULL);
                break;
            case 2:
                pthread_create(&thread, NULL, PNGStats, (void*)filename);
                pthread_join(thread, NULL);
                break;
            case 3:
                pthread_create(&thread, NULL, JPGStats, (void*)filename);
                pthread_join(thread, NULL);
                break;
            case 4:
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
                statsMenu(argv[1]);
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