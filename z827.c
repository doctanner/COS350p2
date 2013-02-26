/* AUTHOR: James Tanner
 * COURSE: COS 350 - Systems Programming
 * PURPOSE: Program 2. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

static const char* FILE_EXT = ".z827";
static const int EXT_LEN = 5;
static const char NON_ASCII_MASK = 0b10000000;
static const char ASCII_MASK = 0b01111111;
static const int IO_BUF_SIZE = 128;

unsigned int compress (int, int);
unsigned int decompress (int, int);

main (int argc, char** argv){
    if (argc !=  2){
      fprintf(stderr,
         "Arguments should be in the form \"%s [file].\n", argv[0]);
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
        // Set source.
        srcPath = malloc(sizeof(char) * (argLen+1));
        strncpy(srcPath, argv[1], argLen);

        // Set dest.
        destPath = malloc(sizeof(char) * (argLen - EXT_LEN + 1));
        strncpy(destPath, argv[1], argLen - EXT_LEN);
        destPath[argLen-EXT_LEN] = 0;

        // Set function to use.
        process = decompress;
    }

    // Open source file.
    fdSource = open(srcPath, O_RDONLY);
    if (fdSource < 0){
        fprintf(stderr, "Could not open source file \"%s\".\n", srcPath);
        exit(-2);
    }

    // Create dest file.
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
		fprintf(stderr, "\tProcessing is complete.\n");
		fprintf(stderr, "\tSource file was not deleted.\n");
		perror("Error code is as follows:\n");
		}
	}
	else{
		fprintf(stderr, "Could not close dest file:\n");
		fprintf(stderr, "\tProcessing may not be complete.\n");
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
   // Report operation.
   printf("Compressing...\n");
   fflush(stdout);

   // Counters:
   int currBytesLoaded = 0;
   unsigned int bytesWritten = 0;
   unsigned int origBytes = 0;
   int bitsOffset = 0;

   // Buffers:
   unsigned char inBuf = 0;
   unsigned char outBuf = 0;
   char *writeBuf = malloc(IO_BUF_SIZE);
   int bytesToWrite = 0;
   char *readBuf = malloc(IO_BUF_SIZE);
   int readBufBytes = 0;
   int readBufIndex = 0;

   // Write space for original size.
   if (write(fdDest, &origBytes, 4) != 4) return 0;

   // Fill read buffer.
   readBufBytes = read(fdSource, readBuf, IO_BUF_SIZE);
   origBytes += readBufBytes;

   // If nothing loaded, fail.
   if (readBufBytes == 0) return 0;

   // Load first char and start compressing.
   inBuf = readBuf[readBufIndex];
   readBufIndex++;
   while (readBufBytes){
      // Fill readBuf if necessary.
      if (readBufIndex >= readBufBytes){
         readBufBytes = read(fdSource, readBuf, IO_BUF_SIZE);
         origBytes += readBufBytes;
         readBufIndex = 0;
         if (readBufBytes == 0){
            readBuf[readBufIndex] = 0;
         }
      }

      // Check for non-ascii char from init or if.
      if (inBuf & NON_ASCII_MASK > 0) return 0;

      /* Copy un-written high-order bits from the in-buffer to 
      * the low-order bits of the out-buffer. */
      /* Note: bitsOffset refers to the number of bits in the current
       * character that have already been written. So this many bits
       * should be "pushed off" the end of the buffer. */
      outBuf = inBuf  >> bitsOffset;

      // Read in the next character.
      inBuf = readBuf[readBufIndex];
      readBufIndex++;

      // If character isn't ASCII, fail.
      if (inBuf & NON_ASCII_MASK > 0) return 0;

	   /* Copy low-order bits from in-buffer to fill unused
	    * high-order bits in out-buffer. */
      /* Note: since bitsOffset is the number of bits "pushed off"
       * the previous character, by left-shifting inBuf by
       * 7 - bitsOffset buts, we can fill the unused high-order
       * bits of outBuf with the same number of bits from inBuf. */
       outBuf = outBuf | (inBuf << (7-bitsOffset));

      // Add byte to writeBuffer
      writeBuf[bytesToWrite] = outBuf;
      bytesToWrite++;

      // If buffer is full...
      if (bytesToWrite == IO_BUF_SIZE){
         // Write out-buffer and count bytes written.
         bytesWritten += write(fdDest,writeBuf,bytesToWrite);

         // Reset buffer
         bytesToWrite = 0;
      }

       // Keep track of shift counts.
       bitsOffset++;

       // If shifted a full seven characters...
       if (bitsOffset > 6){
         // Reset offset.
         bitsOffset = 0;

         // Load next byte, since no useful bits in buffer
         inBuf = readBuf[readBufIndex];
         readBufIndex++;
      }
   }

   // Flush write buffer
   if (bytesToWrite){
      bytesWritten += write(fdDest,writeBuf,bytesToWrite);
   }
   free(writeBuf);
   free(readBuf);

   // Seek to start of file and write original filesize
   if (lseek(fdDest, 0, SEEK_SET) != 0) return 0;
   if (write(fdDest, &origBytes, 4) != 4) return 0;

   // Return total number of bytes written to file.
   return bytesWritten;
}

/* unsigned int decompress(int, int)
 * Attempts to decompress the given source file into the
 * given dest file by reversing the z827 algorithm.
 *
 * On completion, returns the total number of bytes written.
 * If an error is encountered or file cannot be decompressed,
 * returns 0. */
unsigned int decompress (int fdSource, int fdDest){
   // Report operation.
   printf("Decompressing...\n");
   fflush(stdout);

   // Counters:
   int currBytesLoaded = 0;
   unsigned int bytesWritten = 0;
   unsigned int origBytes = 0;
   int bitsInBuf = 0;

   // Buffers:
   /* Note: inBuf is an integer buffer that will hold four
    * bytes of data. inBufNext points to the first byte; as
    * such, it should always contain the next byte to be 
    * decompressed.
    * inBufFill points to the start of the remaining bytes.
    * It us used to refill the buffer from the file.
    * outBuf is used to decompress a char from seven bits
    * to eightand write it to disk.*/
   unsigned int inBuf = 0;
   unsigned char *inBufNext = (char*) &inBuf;
   unsigned char *inBufFill = (char*) &inBuf + 1;
   unsigned char outBuf = 0;
   char *writeBuf = malloc(IO_BUF_SIZE);
   int bytesToWrite = 0;

   // Load original size.
   read(fdSource, &origBytes, 4);
   if (origBytes < 1) return 0;

   // Load initial buffer
   currBytesLoaded = read(fdSource, &inBuf, sizeof(int));
   bitsInBuf = currBytesLoaded * 8;

   // Continue until buffer is empty or all chars written.
   while (bitsInBuf > 0 && bytesWritten + bytesToWrite < origBytes){
      // Decompress char and add it to write buffer.
      outBuf = *inBufNext & ASCII_MASK;
      writeBuf[bytesToWrite] = outBuf;
      bytesToWrite++;

      // If write buffer is full...
      if (bytesToWrite == IO_BUF_SIZE){
         // Write the buffer and reset it.
         bytesWritten += write(fdDest, writeBuf, bytesToWrite);
         bytesToWrite = 0;
      }

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
         bitsInBuf += 8 * read(fdSource, inBufFill, 3);
         inBuf = inBuf >> diff;
      }
   }

   // Flush write buffer
   if (bytesToWrite){
      bytesWritten += write(fdDest, writeBuf, bytesToWrite);
   }

   // Ensure full decompression.
   if (bytesWritten != origBytes){
      fprintf(stderr, "File decryption failed.\n");
      fprintf(stderr, "%u bytes written instead of %u.\n",
         bytesWritten, origBytes);
      return 0;
   }

   // Return total number of bytes written to file.
   return bytesWritten;
}
