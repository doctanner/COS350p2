#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

static char* FILE_EXT = ".z827";
static int EXT_LEN = 5;

void compress (int, int);
void uncompress (int, int);

main (int argc, char** argv){
    if (argc !=  2){
        fprintf(stderr, "Arguments should be in the form \"%s [file].\n", argv[0]);
        exit(-1);
    }

    // File variables:
    char* srcPath = NULL;
    char* destPath = NULL;
    int fdSource = 0;
    int fdDest = 0;

    // Function to run:
    void (*process)(int, int) = NULL;

    // Check if file ends in ".z827"
    int argLen =  strlen(argv[1]);
    int offset = argLen - EXT_LEN;

    // Determine if we must run in compression or decompression mode:
    if (offset < 0 || strcmp(argv[1] + offset, FILE_EXT) != 0){
        printf("Compressing...\n");

        // Set source.
        srcPath = malloc(sizeof(char) * (argLen+1));
        strncpy(srcPath, argv[1], argLen);

        // Set dest.
        destPath = malloc(sizeof(char) * (argLen + EXT_LEN + 1));
        strncpy(destPath, argv[1], argLen);
        strcat(destPath, FILE_EXT);

        // Set function to use.
        process = compress;

    } else{
        printf("Decompressing...\n");

        // Set source.
        srcPath = malloc(sizeof(char) * (argLen+1));
        strncpy(srcPath, argv[1], argLen);

        // Set dest.
        destPath = malloc(sizeof(char) * (argLen - EXT_LEN + 1));
        strncpy(destPath, argv[1], argLen - EXT_LEN);
        destPath[argLen-EXT_LEN] = 0;

        // Set function to use.
        process = uncompress;
    }

    // Open source file.
    printf("Source: %s\n", srcPath);
    fdSource = open(srcPath, O_RDONLY);
    if (fdSource == 0){
        fprintf(stderr, "Could not open source file \"%s\".\n", srcPath);
        exit(-2);
    }

    // Create dest file.
    printf("Dest: %s\n", destPath);
    fdDest = open(destPath, O_WRONLY);
    if (fdDest == 0){
        fprintf(stderr, "Could not open dest file \"%s\".\n", destPath);
        exit(-3);
    }

    // Process the file.
    process(fdSource, fdDest);

    // Close files
    close(fdSource);
    close(fdDest);

    // TODO: Delete source if complete.

    // Free path variables.
    free(srcPath);
    free(destPath);
}

void compress (int fdSource, int fdDest){
    int bytesLoaded = 0;
    int leftShift = 1;
    // Create buffer and load it from file.
    unsigned char inBuf = 0;
    unsigned char outBuf = 0;
    bytesLoaded = read(fdSource, &inBuf, sizeof(char));
    while (bytesLoaded > 0){
        outBuf = inBuf << leftShift;
        bytesLoaded = read(fdSource, &inBuf, sizeof(char));
        outBuf = outBuf | (inBuf >> (7-leftShift));
        write(fdDest,&outBuf,sizeof(char));
        leftShift++;
        if (leftShift > 7){
            leftShift = 1;
            bytesLoaded = read(fdSource, &inBuf, 2);
        }
    }
    outBuf = inBuf << leftShift;
    write(fdDest,&outBuf,sizeof(char));
}

void uncompress (int fdSource, int fdDest){
    fprintf(stderr, "Decompression not yet implemented.\n");
    exit(-4);
}
