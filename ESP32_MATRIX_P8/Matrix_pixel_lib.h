#pragma once

#ifndef __MATRIX_PIXEL_LIB_H
#define __MATRIX_PIXEL_LIB_H


 //Released to the public domain
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "ESP32-VirtualMatrixPanel-I2S-DMA.h" 
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <AnimatedGIF.h>
#include "SPIFFS.h"
#include "FS.h"
#include <TJpg_Decoder.h>

#ifdef INCLUDE_FALLBACK_INDEX_HTM
#include "extras/index_htm.h"
#endif


/*--------------------- MATRIX GPIO CONFIG  -------------------------*/
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19 // Changed from library default
#define C_PIN 5
#define D_PIN -1
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16


/*--------------------- MATRIX PANEL CONFIG -------------------------*/
#define PANEL_RES_X 320      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 16     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 8 // Number of INDIVIDUAL PANELS per ROW

#define SIZE_BOARD 96

#define IMG_WIDTH 64
#define IMG_HEIGHT 48

#define Pannel_Width 32
#define Pannel_Height 16
#define Pannel_para 3

#define P10 3
#define P8 2
#define P4 1



#define FILESYSTEM SPIFFS



typedef enum scrolling_dir_type{
   scr_none=0, 
   scr_left,
   scr_right,
   scr_up,
   scr_down
}scrollingDirection_t;

 enum rotation
 {
	  degre_0=0,
    degre_90=1,
    degre_180=2,
    degre_270=3,
 };

// scrollingDirection_t my_scroling_direction = scr_left;

uint8_t set_rotation (uint8_t  degres);
void DrawPixel(uint16_t row, uint16_t col, uint16_t value);
void GIFDraw(GIFDRAW *pDraw);
void * GIFOpenFile(const char *fname, int32_t *pSize);
void GIFCloseFile(void *pHandle);
int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);
void drawChar_8(int16_t x, int16_t y, uint8_t car, long size, uint16_t value_Text,
                bool bgrd, uint16_t value_Font, int rotation_scrolling);
uint16_t color24to565(uint32_t color24 );
void drawString_8(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t *string, long size, bool warp, 
                   uint16_t value_Text , bool bgrd, uint16_t value_Font,int rotation_scrolling);
void drawString_scr8(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t *string, long size, bool warp, 
                  uint8_t pr, uint8_t pg, uint8_t pb, bool bgrd, uint8_t br, uint8_t bg, uint8_t bb);
void drawChar_scr8(int16_t x, int16_t y, uint8_t car,long size, 
                  uint8_t pr, uint8_t pg, uint8_t pb, bool bgrd, uint8_t br, uint8_t bg, uint8_t bb);
void matrixScrolingDir(scrollingDirection_t dir, uint8_t xy0, uint8_t xy1, uint16_t speed,uint16_t stringlength,uint16_t size);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void dump_text(int16_t x, int16_t y, char* text, long size, uint16_t txt_color, uint16_t bg_color, long time,int rotation_scrolling,uint16_t *array);
void dump_image(char *image_name, long time);
void dump_effet(char *name, long time);
void clearMatrix(void);
void set_Matrix_tab_fullsreem (uint16_t front_color);
void scroll_text(uint8_t ypos,unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB);
bool Matrix_scrolling_Text(uint8_t y0,uint8_t y1,uint8_t x0,uint8_t x1,char *string,uint16_t textcolor,uint16_t bg_color, long size,long Time, int Scrolldirection,int status);
void init_Matrix();
void writePixel_8(int16_t x, int16_t y, uint16_t color) ;
void drawFastVLine_8(int16_t x, int16_t y, int16_t h,uint16_t color);
void writeFastVLine_8(int16_t x, int16_t y, int16_t h,uint16_t color);
void writeLine_8(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);
void drawFastVLine_8(int16_t x, int16_t y, int16_t h,uint16_t color);
void writeFastVLine_8(int16_t x, int16_t y, int16_t h,uint16_t color);
void fillRect_8(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
void writeFillRect_8(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
void scrollChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint16_t dir, uint16_t speed);
void testScrollingChar(const char c, uint16_t speed, uint16_t timeout) ;
void testTextString(uint16_t timeout);
void scrollText(uint16_t X, uint16_t Y,const char *str,uint16_t speed);
void testScrollingText(const char *str, uint16_t speed, uint16_t timeout);
void setFont(const GFXfont *f);
void setScrollDir(uint8_t d );
void setTextFGColor(uint16_t color);
//void drawString_8(int16_t x, int16_t y, unsigned char c,uint16_t width, uint16_t height,bool bgrd, uint16_t color, uint16_t bg, uint8_t size);
//void drawChar_8_(int16_t x, int16_t y, unsigned char c,uint16_t color, uint16_t bg, uint8_t size);

#endif // __MATRIX_PIXEL_LIB_H