/* AUTHOR: James Tanner
 * COURSE: COS 350 - Systems Programming
 * PURPOSE: Program 2. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

static char* FILE_EXT = ".z827";
static int EXT_LEN = 5;
static char NON_ASCII_MASK = 0b10000000;
static char ASCII_MASK = 0b01111111;

unsigned int compress (int, int);
unsigned int uncompress (int, int);

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
    unsigned int (*process)(int, int) = NULL;

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
    if (fdSource < 0){
        fprintf(stderr, "Could not open source file \"%s\".\n", srcPath);
        exit(-2);
    }

    // Create dest file.
    printf("Dest: %s\n", destPath);
    fdDest = creat(destPath, 0644);
    if (fdDest < 0){
        fprintf(stderr, "Could not open dest file \"%s\".\n", destPath);
        exit(-3);
    }

    // Process the file.
    unsigned int result = process(fdSource, fdDest);
    if (result == 0){
    	// Compression or decompression failed.
		fprintf(stderr, "File could not be processed:\n");
		fprintf(stderr, "\tCorrupt dest file removed.\n");
		fprintf(stderr, "\tSource file was not deleted.\n");
		unlink(destPath);
		exit(-4);
    }

    // Close files
    close(fdSource);
    if (close(fdDest) == 0){
    	// Dest closed successfully. Remove source.
		if (unlink(srcPath)!=0){
		fprintf(stderr, "Could not remove source file:\n");
		fprintf(stderr, "\tCompression or decompression is complete.\n");
		fprintf(stderr, "\tSource file was not deleted.\n");
		perror("Error code is as follows:\n");
		}
	}
	else{
		fprintf(stderr, "Could not close dest file:\n");
		fprintf(stderr, "\tCompression or decompression may not be complete.\n");
		fprintf(stderr, "\tSource file was not deleted.\n");
		perror("Error code is as follows:\n");
	}

	// Report.
	printf("Done.\n");

    // Free path variables.
    free(srcPath);
    free(destPath);
}

/* unsigned int compress(int, int)
 * Attempts to compress the given source file into the
 * given dest file using the z827 algorithm.
 *
 * On completion, returns the total number of bytes written.
 * If an error is encountered or file cannot be compressed,
 * returns 0. */
unsigned int compress (int fdSource, int fdDest){
	// Counters:
    int currBytesLoaded = 0;
    unsigned int bytesWritten = 0;
    unsigned int origBytes = 0;
    int bitsOffset = 0;

    // Buffers:
    unsigned char inBuf = 0;
    unsigned char outBuf = 0;

    // Write space for original size.
	if (write(fdDest, &origBytes, 4) != 4) return 0;

	// Load first char and start compressing.
    currBytesLoaded = read(fdSource, &inBuf, sizeof(char));
    origBytes += currBytesLoaded;
    while (currBytesLoaded > 0){
		// Check for non-ascii char from init or if.
	  	if (inBuf & NON_ASCII_MASK > 0) return -1;

    	/* Copy un-written high-order bits from the in-buffer to 
    	 * the low-order bits of the out-buffer. */
    	/* Note: bitsOffset refers to the number of bits in the current
    	 * character that have already been written. So this many bits
    	 * should be "pushed off" the end of the buffer. */
        outBuf = inBuf  >> bitsOffset;

        // Read in the next character.
	    currBytesLoaded = read(fdSource, &inBuf, sizeof(char));
    	origBytes += currBytesLoaded;
        if (currBytesLoaded < 1) inBuf = 0;
	  	else if (inBuf & NON_ASCII_MASK > 0) return -1;

		/* Copy low-order bits from in-buffer to fill unused
		 * high-order bits in out-buffer. */
		/* Note: since bitsOffset is the number of bits "pushed off"
		 * the previous character, by left-shifting inBuf by
		 * 7 - bitsOffset buts, we can fill the unused high-order
		 * bits of outBuf with the same number of bits from inBuf. */
        outBuf = outBuf | (inBuf << (7-bitsOffset));

        // Write out-buffer and count bytes written.
        bytesWritten += write(fdDest,&outBuf,sizeof(char));

        // Keep track of shift counts.
        bitsOffset++;
        if (bitsOffset > 6){
            bitsOffset = 0;
		    currBytesLoaded = read(fdSource, &inBuf, sizeof(char));
	    	origBytes += currBytesLoaded;
        }
	}

	// Seek to start of file and write original filesize
	if (lseek(fdDest, 0, SEEK_SET) != 0) return 0;
	if (write(fdDest, &origBytes, 4) != 4) return 0;

	// Return total number of bytes written to file.
	return bytesWritten;
}

/* unsigned int uncompress(int, int)
 * Attempts to uncompress the given source file into the
 * given dest file by reversing the z827 algorithm.
 *
 * On completion, returns the total number of bytes written.
 * If an error is encountered or file cannot be uncompressed,
 * returns 0. */
unsigned int uncompress (int fdSource, int fdDest){
	// Counters:
    int currBytesLoaded = 0;
    unsigned int bytesWritten = 0;
    unsigned int origBytes = 0;
    int bitsInBuf = 0;

    // Buffers:
    /* Note: inBuf is an integer buffer that will hold four
     * bytes of data. inLbits points to the first byte; as
     * such, it should always contain the next byte to be 
     * uncompressed.
     * inHBits points to the start of the remaining bytes.
     * It us used to refill the buffer from the file.
     * outBuf is used to decompress a char from seven bits
     * to eightand write it to disk.*/
    unsigned int inBuf = 0;
    unsigned char *inLbits = (char*) &inBuf;
    unsigned char *inHbits = (char*) &inBuf + 1;
    unsigned char outBuf = 0;

    // Load original size.
    read(fdSource, &origBytes, 4);
    if (origBytes < 1) return 0;

    // Load initial buffer
    currBytesLoaded = read(fdSource, &inBuf, sizeof(int));
    bitsInBuf = currBytesLoaded * 8;

    // Continue until buffer is empty or all chars written.
    while (bitsInBuf > 0 && bytesWritten < origBytes){
    	// Decompress char and write it.
    	outBuf = *inLbits & ASCII_MASK;
		bytesWritten += write(fdDest, &outBuf, sizeof(char));

		// Shift buffer to right-align next byte.
		inBuf = inBuf >> 7;
		bitsInBuf -= 7;

		// If buffer low, refill it.
   		if (bitsInBuf <= 8){
   			/* To refull the buffer, we shift it to left-align
   			 * the remaining bits within the low-end byte. This
   			 * prevents junk data from entering the bit-stream.
   			 * Then, load the three high-end bytes from the file.
   			 * Finally, shift the entire buffer right in order
   			 * to return the buffer to its normal right-aligned
   			 * state. */
   			int diff = 8 - bitsInBuf;
   			inBuf = inBuf << diff;
   			bitsInBuf += 8 * read(fdSource, inHbits, 3);
   			inBuf = inBuf >> diff;
   		}
	}

	// Ensure full uncompression.
	if (bytesWritten != origBytes) return 0;

	// Return total number of bytes written to file.
	return bytesWritten;
}
