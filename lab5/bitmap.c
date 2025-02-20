#include <lcom/lcf.h>
#include <lcom/lab5.h>
#include <machine/int86.h>
#include <kbc.h>
#include <kbd.h>
#include <liblm.h>
#include <stdint.h>
#include <stdio.h>
#include "bitmap.h"
#include "vbe1.h"
#include <stdlib.h>
//#include <lcom/video_gr.h>
#include <sys/mman.h>
#include "graphicalmacros.h"
#include "i8042.h"
#include "i8254.h"
#include "sprite.h"
#include "bitmap.h"


#define FIRST_BYTE_GREEN_COLOR  0xE0 // necessário a extensão devido ao sinal negativo
#define SECOND_BYTE_GREEN_COLOR 0x07

Bitmap* loadBitmap(const char* filename) {
    // allocating necessary size
    Bitmap* bmp = (Bitmap*) malloc(sizeof(Bitmap));

    // open filename in read binary mode
    FILE *filePtr;
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL)
        return NULL;

    // read the bitmap file header
    BitmapFileHeader bitmapFileHeader;
    fread(&bitmapFileHeader, 2, 1, filePtr);

    // verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.type != 0x4D42) {
        fclose(filePtr);
        return NULL;
    }

    int rd;
    do {
        if ((rd = fread(&bitmapFileHeader.size, 4, 1, filePtr)) != 1)
            break;
        if ((rd = fread(&bitmapFileHeader.reserved, 4, 1, filePtr)) != 1)
            break;
        if ((rd = fread(&bitmapFileHeader.offset, 4, 1, filePtr)) != 1)
            break;
    } while (0);

    if (rd != 1) {
        fprintf(stderr, "Error reading file\n");
        exit(-1);
    }

    // read the bitmap info header
    BitmapInfoHeader bitmapInfoHeader;
    fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, filePtr);

    // move file pointer to the begining of bitmap data
    fseek(filePtr, bitmapFileHeader.offset, SEEK_SET);

    // allocate enough memory for the bitmap image data
    unsigned char* bitmapImage = (unsigned char*) malloc(
            bitmapInfoHeader.imageSize);

    // verify memory allocation
    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    // read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader.imageSize, 1, filePtr);

    // make sure bitmap image data was read
    if (bitmapImage == NULL) {
        fclose(filePtr);
        return NULL;
    }

    // close file and return bitmap image data
    fclose(filePtr);

    bmp->bitmapData = bitmapImage;
    bmp->bitmapInfoHeader = bitmapInfoHeader;

    return bmp;
}

void drawBitmap(Bitmap* bmp, int x, int y, Alignment alignment,char *video_mem) {
    if (bmp == NULL)
        return;

    int width = bmp->bitmapInfoHeader.width;
    int drawWidth = width;
    int height = bmp->bitmapInfoHeader.height;

    if (alignment == ALIGN_CENTER)
        x -= width / 2;
    else if (alignment == ALIGN_RIGHT)
        x -= width;

    if (x + width < 0 || x >get_h_res() || y + height < 0
            || y > get_v_res())
        return;

    int xCorrection = 0;
    if (x < 0) {
        xCorrection = -x;
        drawWidth -= xCorrection;
        x = 0;

        if (drawWidth >get_h_res())
            drawWidth =get_h_res();
    } else if (x + drawWidth >=get_h_res()) {
        drawWidth =get_h_res() - x;
    }

    char* bufferStartPos;
    unsigned char *imgStartPos;


    uint8_t firstbyte = 0x1f; // necessário a extensão devido ao sinal negativo
    uint8_t secontbyte =  0xf8;

    int i;
    for (i = 0; i < height; i++) {
        int pos = y + height - 1 - i;

        if (pos < 0 || pos >= get_v_res())
            continue;

        bufferStartPos = video_mem;
        bufferStartPos += x * 2 + pos *get_h_res()*2;

        imgStartPos = bmp->bitmapData + xCorrection * 2 + i * width * 2;

	    int j;
		for (j = 0; j < drawWidth * 2; j = j + 2) {

			if (imgStartPos[j] != firstbyte && imgStartPos[j+1]!= secontbyte) {
				bufferStartPos[j] = imgStartPos[j];
				bufferStartPos[j + 1] = imgStartPos[j + 1];
			}

            
            
            
}
    }
}

void deleteBitmap(Bitmap* bmp) {
    if (bmp == NULL)
        return;

    free(bmp->bitmapData);
    free(bmp);
}
