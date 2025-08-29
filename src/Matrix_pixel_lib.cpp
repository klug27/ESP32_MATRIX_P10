#include "matrix_pixel_lib.h"
#include "fonts.h"
//#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>

#include "glcdfont.c"


#define CONFIG_FILE "/jsonFiles/config.json"
#define SCENARIO_FILE "/jsonFiles/scenario.json"
#define USER_CONFIG_FILE "/jsonFiles/userconfig.json"
#define GENERIC_CONFIG_FILE "/jsonFiles/genericconfig.json"
#define SECRET_FILE "/jsonFiles/secret.json"
#define EFFET_FOLDER "/Effets"
#define EFFET_FILE "/jsonFiles/effets.json"
#define GIF_FOLDER "/Effets/"
#define VIDEO_FOLDER "/Videos"
#define VIDEO_FILE"/jsonFiles/videos.json"
#define IMAGE_FOLDER "/Images"
#define IMAGE_FILE"/jsonFiles/images.json"
#define ANIMATION_FILE"/jsonFiles/animation.json"
#define FONT_FILE"/jsonFiles/font.json" 
#define Waiting_time 2000
#define LED_BUILTIN 2

AnimatedGIF gif;
File f;

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {
#ifdef __AVR__
  return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
  // expression in __AVR__ section may generate "dereferencing type-punned
  // pointer will break strict-aliasing rules" warning In fact, on other
  // platforms (such as STM32) there is no need to do this pointer magic as
  // program memory may be read in a usual way So expression may be simplified
  return gfxFont->glyph + c;
#endif //__AVR__
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {
#ifdef __AVR__
  return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
#else
  // expression in __AVR__ section generates "dereferencing type-punned pointer
  // will break strict-aliasing rules" warning In fact, on other platforms (such
  // as STM32) there is no need to do this pointer magic as program memory may
  // be read in a usual way So expression may be simplified
  return gfxFont->bitmap;
#endif //__AVR__
}

uint16_t u16TextBuffer[24*5*SIZE_BOARD];
uint16_t *tabpixels=NULL;
//uint16_t *tabpixels = (uint16_t*)malloc(sizeof(uint16_t[IMG_HEIGHT][SIZE_BOARD*5])); 
int x_offset, y_offset;
int16_t cursor_x, cursor_y;   // Cursor position
uint8_t size_x, size_y;       // Font size Multiplier default = 1 => 5x7 Font (5width,7Height)
uint16_t textFGColor, textBGColor,rotation=degre_0;
uint8_t dir ;
uint16_t Scr_tab_width=0, Scr_tab_height=0;
bool scenario_read=false;
int Panel_type=P10; // Insert your pannel type
extern bool NEW_SCENARIO;
extern int Brightness;

// Module configuration
HUB75_I2S_CFG mxconfig(
	PANEL_RES_X*2,   // module width
	PANEL_RES_Y/2,   // module height
	PANEL_CHAIN    // Chain length
);

MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel *virtualDisp = nullptr;

unsigned long start_tick = 0;

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
      iWidth = MATRIX_WIDTH;

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + pDraw->iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < pDraw->iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        if (iCount) // any opaque pixels?
        {
          for(int xOffset = 0; xOffset < iCount; xOffset++ ){
            DrawPixel(x + xOffset,y,usTemp[xOffset]);
            //dma_display->drawPixel(x + xOffset, y, usTemp[xOffset]); // 565 Color Format
          }
          x += iCount;
          iCount = 0;
        }
        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    }
    else // does not have transparency
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<pDraw->iWidth; x++)
      {
        DrawPixel(x,y,usPalette[*s++]);
        //dma_display->drawPixel(x, y, usPalette[*s++]); // color 565
      }
    }
} /* GIFDraw() */


void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  Serial.print("Playing gif: ");
  Serial.println(fname);
  f = FILESYSTEM.open(fname);
  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */


void dump_effet(char *name, long time)
{
  start_tick = millis();
  do{   
      if(NULL != strstr(name, ".gif"))
      {
        if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
        {
          x_offset = (MATRIX_WIDTH - gif.getCanvasWidth())/2;
          if (x_offset < 0) x_offset = 0;
          y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight())/2;
          if (y_offset < 0) y_offset = 0;
          Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
          Serial.flush();
          while ((gif.playFrame(true, NULL))/*&&(scenario_read==true)*/)
          {      
            if ( (millis() - start_tick) > time * 1000) { // we'll get bored after about 1 seconds of the same looping gif
              break;
            }
          }
          gif.close();
        }
        else
        {
            Serial.println("no file type expeted"); 
          break;
        }
      }
      else
      {
          Serial.println("no file type expeted"); 
       break;
      }
  }while((millis() - start_tick) < time * 1000); 
} /* ShowGIF() */


void writePixels_16(int16_t x, int16_t y, int16_t w, int16_t h, int16_t len, uint16_t *pc){

  uint16_t i=0, j=0; 
  if((x<0) || (x>IMG_WIDTH) || (y<0) || (y>IMG_HEIGHT)){
    //MAIN_LOGDEBUG4("bad coordinates '%d x %d' where Matrix size is '%d x %d'\n", x, y, IMG_WIDTH, IMG_HEIGHT);
  }
  else{ 
    for(j=0; j<h; j++){
      if(((y+j)>=IMG_HEIGHT) ){break;}      
      for(i=0; i<w; i++){
        if((x+i)>=IMG_WIDTH){break;}
        DrawPixel((x+i), (y+j), pc[i + j*w]);
       // img[(x+i) + (y+j)*IMG_WIDTH]= pc[i + j*w];
      }
    }
  }
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= IMG_HEIGHT ) return 0;
   writePixels_16(x, y, w, h, w*h, bitmap);
  return 1;
}

void dump_image(char *image_name, long time)
{
  uint16_t Time_delay=time * 1000;
  if((NULL != strstr(image_name, ".jpg")) || (NULL != strstr(image_name, ".jpeg")))
  {
      char path[255];
      memset(path, 0, 255);
     // strcat(path, IMAGE_FOLDER);
      strcat(path, image_name);
      Serial.printf("path : %s\n", path);
          
      // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
      TJpgDec.setJpgScale(1);
      // The decoder must be given the exact name of the rendering function above
      TJpgDec.setCallback(tft_output);

      // Get the width and height in pixels of the jpeg if you wish
      uint16_t w = 0, h = 0;
      TJpgDec.getSdJpgSize(&w, &h, path);
      //Serial.printf(F("Width = ")); MAIN_LOGDEBUG0(F(w)); MAIN_LOGDEBUG0(F(", height = ")); MAIN_LOGDEBUG0(F(h));MAIN_LOGDEBUG();
  //    unsigned long start_tick = millis();  
      // Draw the image, top left at 0,0
      TJpgDec.drawSdJpg(0, 0, path); 
      while((scenario_read==true)&&(Time_delay>0))
      {delay(1);Time_delay--;}
  }
  else
  {
     // put here others image type 
    Serial.println("no image file type expeted"); 
  } 
}


/*
void drawString_8(int16_t x, int16_t y, unsigned char c,uint16_t width, uint16_t height,bool bgrd, uint16_t color, uint16_t bg, uint8_t size) 
{

  if (!gfxFont) { // 'Classic' built-in font

    if ((x >= width) ||              // Clip right
        (y >= height) ||             // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
      return;

    if (!bgrd && (c >= 176))
      c++; // Handle 'classic' charset behavior

    for (int8_t i = 0; i < 5; i++) 
	{ // Char bitmap = 5 columns
      uint8_t line = pgm_read_byte(&font[c * 5 + i]);
      for (int8_t j = 0; j < 8; j++, line >>= 1) 
	  {
        if (line & 1) 
		{
          if (size == 1 && size == 1)
           //tabpixels[(y + j * SIZE_BOARD*6) + x + i+IMG_WIDTH]=color;
            writePixel_8(x + i, y + j, color);
          else           
            writeFillRect_8(x + i * size, y + j * size, size, size,
                          color);
        } else if (bg != color) 
		{
          if (size == 1 && size == 1)
           // tabpixels[(y + j * SIZE_BOARD*6) + x + i+IMG_WIDTH]=bg;
            writePixel_8(x + i, y + j, bg);
          else
            writeFillRect_8(x + i * size, y + j * size, size, size, bg);
        }
      }
    }
    if (bg != color) { // If opaque, draw vertical line for last column
      if (size == 1 && size == 1)
        writeFastVLine_8(x + 5, y, 8, bg);
      else
        writeFillRect_8(x + 5 * size, y, size, 8 * size, bg);
    }

  } else 
  { // Custom font

    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling
    // drawChar() directly with 'bad' characters of font may cause mayhem!

    c -= (uint8_t)pgm_read_byte(&gfxFont->first);
    GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
    uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
    uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
    int8_t xo = pgm_read_byte(&glyph->xOffset),
           yo = pgm_read_byte(&glyph->yOffset);
    uint8_t xx, yy, bits = 0, bit = 0;
    int16_t xo16 = 0, yo16 = 0;

    if (size > 1 || size > 1) {
      xo16 = xo;
      yo16 = yo;
    }
    for (yy = 0; yy < h; yy++) {
      for (xx = 0; xx < w; xx++) 
	  {
        if (!(bit++ & 7)) {
          bits = pgm_read_byte(&bitmap[bo++]);
        }
        if (bits & 0x80) 
		{
          if (size == 1 && size == 1) {
            //tabpixels[(y + yo + yy * SIZE_BOARD*6) + x + xo + xx+IMG_WIDTH]=color;
            writePixel_8(x + xo + xx, y + yo + yy, color);
          } else 
		      {
            writeFillRect_8(x + (xo16 + xx) * size, y + (yo16 + yy) * size,
                          size, size, color);
          }
        }
        bits <<= 1;
      }
    }
  } // End classic vs custom font
}*/

void testScrollingChar(const char c, uint16_t speed, uint16_t timeout) 
{
  Serial.println("Scrolling Char");
  uint16_t myFGColor = dma_display->color565(180,0,0);
  uint16_t myBGColor = dma_display->color565(60,120,0);
  dma_display->fillScreen(dma_display->color444(0,0,0));
  dma_display->setTextWrap(true);
  // from right to left with wrap
  scrollChar(58,5,c, myFGColor, myFGColor, 1, speed);
  // left out with wrap
  delay(500);
  scrollChar(0,5,c, myBGColor, myBGColor, 1, speed);
  delay(timeout);
}

void setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}
void setScrollDir(uint8_t d = 1) { dir = (d != 1) ? 0 : 1;};    // set scroll dir default = 1
void setTextFGColor(uint16_t color) {textFGColor = color;};
void setTextBGColor(uint16_t color) {textBGColor = color;};

void testTextString(uint16_t timeout) 
{
  dma_display->fillScreen(dma_display->color444(0,0,0));
  setTextFGColor(dma_display->color565(0,60,255));
  dma_display->setCursor(0,5);
  dma_display->write("HURRA");
  delay(timeout);
}

GFXfont *gfxFont;

void setFont(const GFXfont *f) {
  if (f) {          // Font struct pointer passed in?
    if (!gfxFont) { // And no current font struct?
      // Switching from classic to new font behavior.
      // Move cursor pos down 6 pixels so it's on baseline.
      cursor_y += 6;
    }
  } else if (gfxFont) { // NULL passed.  Current font struct defined?
    // Switching from new to classic font behavior.
    // Move cursor pos up 6 pixels so it's at top-left of char.
    cursor_y -= 6;
  }
  gfxFont = (GFXfont *)f;
}



void scrollChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint16_t dir, uint16_t speed)
{
    if ((x >= IMG_WIDTH) ||
    (y >= IMG_HEIGHT) ||
    ((x + 6 * size_x-1) < 0) ||
    ((y + 8 * size_y-1) <0))
    return; 

    dma_display->setTextWrap(true); 

      // loop s = scroll-loop, scrolls char 5 pixels into dir
  uint8_t lastX = x;
  for (int8_t s = 0; s < IMG_WIDTH; s++) 
  {
    Serial.printf("X:%d ", x);
    // clear current position
    dma_display->fillRect(x,y,5,7,0);
    x = lastX - s;
    for (int8_t i = 0; i < 5; i++) 
    {
      uint8_t line = pgm_read_byte(&font[c * 5 + i]); 
      Serial.printf("i:%d ", i);
      // ignore all pixels outside panel
      if (x+i >= IMG_WIDTH) continue;
      for (int8_t j=0; j < 8; j++, line >>= 1) 
      {
        if (line & 1)
        {
          
          Serial.printf(" ON %d", x+i);
          // we read 1
          if (x >= 0) 
          {
            DrawPixel(x+i, y+j, color);
          }
          else if (x+i >= 0)
          {
            DrawPixel(x+i, y+j, color);
          }
        }
        else if (bg != color) 
        {
         // we read 0
          Serial.printf(" OFF %d", x+i);
          if (x >= 0) {
            DrawPixel(x+i, y+j, bg);
          }
          else if (x+i >= 0) {
            DrawPixel(x+i, y+j, bg);
          }    
        }
      }
    }
    Serial.printf("\n");
    delay(speed);
    dma_display->fillScreen(dma_display->color444(0, 0, 0));
  }
}


void testScrollingText(const char *str, uint16_t speed, uint16_t timeout) 
{

  Serial.println("Scrolling Text as loop");
  // pre config
  uint16_t red = dma_display->color565(255,0,100);
  uint16_t blue100 = dma_display->color565(0,0,100);
  uint16_t black = dma_display->color565(0,0,0);
  uint16_t green = dma_display->color565(0,255,0);
  uint16_t green150 = dma_display->color565(0,150,0);

  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setCursor(31,5);
  setScrollDir(1);

  /** black background **/
  setTextFGColor(green150);
  scrollText(0,0,"** Welcome **", speed);
  dma_display->fillScreen(black);
  delay(timeout / 2) ;

  /** scrolling with colored background */
  //dma_display->fillRect(0,4,IMG_WIDTH,8,blue100);
  // scrolling, using default pixels size = length of string (not used parameter pixels)
  setTextFGColor(red);
  setTextBGColor(blue100);
  scrollText(0,0,str, speed);
  delay(timeout / 2) ;

  // same as above but now from left to right
  setScrollDir(0);
  setTextFGColor(blue100);
  setTextBGColor(red);
  //dma_display->fillRect(0,4,IMG_WIDTH,8,red);
  scrollText(0,0,str, speed);

  delay(timeout);
  dma_display->fillScreen(black);
  setTextFGColor(red);
}


void scrollText(uint16_t X, uint16_t Y,const char *str,uint16_t speed) 
{
  uint16_t pixels=0;
  const uint8_t xSize = 6;
  uint16_t len = strlen(str);
  uint8_t array[len * xSize];         // size of array number of chars * width of char
  uint16_t aPtr = 0;
  uint16_t t;
  uint8_t car=24;
  car = car - ' ';  // caracter value in ascii table - 32   


  // generate array
  char c = *str;
  // Serial.printf("size *str (%d), size array: (%d) \n", len, lenArray);

  while (c) 
  {
    for (int8_t i = 0; i < 5; i++) 
    {
      uint8_t line = pgm_read_byte(&font[c * 5 + i]); 
      array[aPtr++] = line;
    }
    str++;
    c = *str;
    array[aPtr++] = 0x00;     // line with 0 (space between chars)
  }
  array[aPtr++] = 0x00;     // line with 0 (space between chars)

  int16_t x,y,lastX, p;
  lastX = (dir) ? IMG_WIDTH : 0;
  x = cursor_x;
  y = cursor_y;
  Serial.printf("X: %d, Y: %d \n", x,y);
  p=0;
  pixels = (pixels) ? pixels : len * xSize;

  while (p <= pixels) 
  {
    // remove last pixel positions
    dma_display->fillRect(x,y,5,7,textBGColor);
    // set new pixel position
    x = (dir) ? lastX - p : lastX + p - pixels;
    // iterator through our array
    for (uint8_t i=0; i < (len*xSize); i++)
    {
      uint8_t line = array[i];
      for (uint8_t j=0; j < 8; j++, line>>=1) 
      {
        if (line & 1) 
        {
          // got 1, if x + i outside panel ignore pixel
          if (x + i >= 0 && x + i < IMG_WIDTH) 
          {
            DrawPixel(x + i, y + j+Y, textFGColor);
          }
        }
        else 
        {
          // got 0
          if (x + i >= 0 && x + i < IMG_WIDTH) {
            DrawPixel(x + i, y + j+Y, textBGColor);
          }
        } // if
      } // for j
    } // for i
    p++;
    delay(speed);
  } // while 
}


void drawChar_8(int16_t x, int16_t y, uint8_t car, long size, uint16_t value_Text,
               bool bgrd, uint16_t value_Font, int rotation_scrolling)
{

  uint16_t temp, t1, t, pos, mode=1;
  uint16_t y0 = y;
  uint16_t csize = ((size/8) + ((size % 8) ? 1 : 0)) * (size / 2);
  car = car - ' ';  // caracter value in ascii table - 32   

  if (size == 8)
  {
      for(pos=0;pos<8;pos++)
      {
          temp = asc2_0806[car][pos];//Call 0806 font, you need to take the model definition
          for(t=0;t<6;t++){                 
              if(temp&0x80)
              {
                if((rotation_scrolling==1)||(rotation_scrolling==2))
                  tabpixels[(y+pos * SIZE_BOARD*5) + x+t+IMG_WIDTH]=value_Text;
                else
                  tabpixels[(y+pos * SIZE_BOARD*5) + x+t]=value_Text;      
               }
              else if (bgrd)
              {
                 if((rotation_scrolling==1)||(rotation_scrolling==2))
                  tabpixels[(y+pos * SIZE_BOARD*5) + x+t+IMG_WIDTH]=value_Font;
                 else
                  tabpixels[(y+pos * SIZE_BOARD*5) + x+t]=value_Font;
              }
              else
              {
                if((rotation_scrolling==1)||(rotation_scrolling==2))
                   tabpixels[(y+pos * SIZE_BOARD*5) + x+t+IMG_WIDTH]=0;
                else
                  tabpixels[(y+pos * SIZE_BOARD*5) + x+t]=0;
              }
              temp<<=1;
          }
       } 
  }
  else
  {
      for (t = 0; t < csize; t++)
      {
        if (size == 8)      temp = asc2_0806[car][t]; // not necessary 
        else if (size == 12)temp = asc2_1206[car][t];
        else if (size == 16)temp = asc2_1608[car][t];
        else if (size == 24)temp = asc2_2412[car][t];
        else return;
        for (t1 = 0; t1 < 8; t1++)
        {
          if (temp & 0x80) 
          {
            if((rotation_scrolling==1)||(rotation_scrolling==2))
              tabpixels[(y * SIZE_BOARD*5) + x+IMG_WIDTH]=value_Text;
            else
              tabpixels[(y * SIZE_BOARD*5) + x]=value_Text;
          }
          else if (bgrd)
          {
            if((rotation_scrolling==1)||(rotation_scrolling==2))
             tabpixels[(y * SIZE_BOARD*5) + x+IMG_WIDTH]=value_Font;
            else
              tabpixels[(y * SIZE_BOARD*5) + x]=value_Font;
          }
          else
          {
            if((rotation_scrolling==1)||(rotation_scrolling==2))
              tabpixels[(y * SIZE_BOARD*5) + x+IMG_WIDTH]=0;
            else
              tabpixels[(y * SIZE_BOARD*5) + x]=0;
          }
          temp <<= 1;
          y++;
          if (y >= (SIZE_BOARD*5))return;
          if ((y - y0) == size)
          {
            y = y0;
            x++;
            if (x >= (SIZE_BOARD*5)) return;
            break;
          }
        }
      }
  }
}
void color565to888(const uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b){
  r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
  g = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
  b = (((color & 0x1F) * 527) + 23) >> 6;
}

uint16_t color888to565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

uint16_t color24to565(uint32_t color24 ) 
{
  uint8_t r=0; uint8_t g=0; uint8_t b=0;
  r= color24>>16;
  g= color24>>8;
  b= color24;
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

void drawString_8(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t *string, long size, bool warp, 
                  uint16_t value_Text , bool bgrd, uint16_t value_Font,int rotation_scrolling)
{
    uint16_t x0 = x;
    width += x;
    height += y;
    while ((*string <= '~') && (*string >= ' '))
    {
      if (x >= width && warp) 
      {
        x = x0;
        y += size;
      }
      if (y >= height)break;
      drawChar_8(x,y,*string,size,value_Text,bgrd,value_Font,rotation_scrolling);
      x += size/2;
      string++;
    }               
 }

void dump_text(int16_t x, int16_t y, char* text, long size, uint16_t txt_color, uint16_t bg_color, long time,int rotation_scrolling)
{
    uint16_t Time_delay=time * 1000;
    uint8_t tr=0, tg=0, tb=0, br=0, bg=0, bb=0;
    drawString_8(x,y,SIZE_BOARD*5, 24,(uint8_t*)text, size, true, txt_color, true, bg_color,rotation_scrolling);    
    //drawString_8(0,10, *text,SIZE_BOARD*5, IMG_HEIGHT,true, txt_color, bg_color, 1);
    while(Time_delay>0)
    {delay(1);Time_delay--;}
}

void clearMatrix(void)
{
  dma_display->clearScreen();
  memset(&u16TextBuffer[0], 0, sizeof(u16TextBuffer));
}

void drawPixel_scr8(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b){
   tabpixels[x + y*Scr_tab_width] = color888to565(r, g, b);
}

void set_Matrix_tab_fullsreem (uint16_t front_color)
{
  for (uint8_t i=0;i<24;i++)
   for (uint16_t j=0;j<SIZE_BOARD*5;j++)
     {
       tabpixels[(i * SIZE_BOARD*5) + j]=front_color;
  	 }
}

void matrixScrolingDir(scrollingDirection_t dir, uint8_t xy0, uint8_t xy1, uint16_t speed)
{
   uint16_t x,y, i; 
  /* switch(dir){
     case scr_left: 
      for (i=xy0; i<=xy1; i++)
        {   
              tabpixels[Scr_tab_width-1 + i * Scr_tab_width] = img[0 + i * IMG_WIDTH];
          }
          for (y=xy0; y<=xy1; y++)
          {
            for (x=0; x<IMG_WIDTH; x++)
            {
              img[x + y * IMG_WIDTH] = img[x + y * IMG_WIDTH + 1]; 
            }
            for (x=0; x<Scr_tab_width; x++)
            {
              tabpixels[x + y * Scr_tab_width] = tabpixels[x + y * Scr_tab_width + 1];
            } 
          } 
          for (i=xy0; i<=xy1; i++)
          {
            img[IMG_WIDTH-1 + i * IMG_WIDTH] = tabpixels[0 + i * Scr_tab_width]; 
          }
          break; 
         
          case scr_right:
          // right scroling
            for (i=xy0; i<=xy1; i++)
            {    
              tabpixels[0 + i * Scr_tab_width] = img[IMG_WIDTH-1 + i * IMG_WIDTH];
            }
            for (y=xy0; y<=xy1; y++)
            {
              for (x=0; x < IMG_WIDTH; x++)
              { 
              img[((IMG_WIDTH-1)-x) + y * (IMG_WIDTH)] = img[((IMG_WIDTH-2)-x) + y * (IMG_WIDTH)]; 
              }
              for (x=0; x < Scr_tab_width; x++)
              { 
              tabpixels[((Scr_tab_width-1)-x) + y*(Scr_tab_width)] = tabpixels[(((Scr_tab_width)-2)-x) + y*(Scr_tab_width)];    
              }
            } 
            for (i=xy0; i<=xy1; i++)
            {
                img[0 + i * IMG_WIDTH] = tabpixels[Scr_tab_width-1 + i * Scr_tab_width]; 
            }
            break;
  
           case scr_up :
           // up scroling
              for (i=xy0; i<=xy1; i++)
              {    
                 tabpixels[i + (Scr_tab_height-1) * Scr_tab_width] = img[i + 0 * IMG_WIDTH];
              }
              
              for (y=0; y<IMG_HEIGHT; y++)
              {
                for (x=xy0; x <=xy1; x++)
                { 
                  img[x + y * IMG_WIDTH] = img[x + (y+1) * IMG_WIDTH];      
                }
              }
              for (y=0; y<Scr_tab_height; y++)
              {
                for (x=xy0; x <=xy1; x++)
                {
                  tabpixels[x + y * Scr_tab_width] = tabpixels[x + (y+1) * Scr_tab_width];                     
                }
              } 
              
              for (i=xy0; i<=xy1; i++)
              {
                img[i + (IMG_HEIGHT-1) * IMG_WIDTH] = tabpixels[i + 0 * Scr_tab_width]; 
              }
            break;
      
            case scr_down : 
           //  down scroling
              for (i=xy0; i<=xy1; i++)
              {    
                tabpixels[i + 0 * Scr_tab_width] = img[i + (IMG_HEIGHT-1) * IMG_WIDTH];
              }
              for (y=0; y<IMG_HEIGHT; y++)
              {
                for (x=xy0; x <=xy1; x++)
                { 
                  img[x + ((IMG_HEIGHT-1)-y) * IMG_WIDTH] = img[x + ((IMG_HEIGHT-2)-y) * IMG_WIDTH];          
                }
              }
              for (y=0; y<Scr_tab_height; y++)
              {
                for (x=xy0; x <=xy1; x++)
                { 
                 tabpixels[x + ((Scr_tab_height-1)-y)*Scr_tab_width] = tabpixels[x + ((Scr_tab_height-2)-y) * Scr_tab_width];   
                }
              } 
              for(i=xy0; i<=xy1; i++)
              {
                 img[i  + 0 * IMG_WIDTH] = tabpixels[i + ((Scr_tab_height)-1)*Scr_tab_width];                
              }
              break; 

          default:
                break;
   }*/
   delay(speed);
}

 void drawString_scr8(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t *string, long size, bool warp, 
                  uint8_t pr, uint8_t pg, uint8_t pb, bool bgrd, uint8_t br, uint8_t bg, uint8_t bb){

    uint16_t x0 = x;
    width += x;
    height += y;
    //  HAL_TIM_Base_Stop_IT(&htim2);
    while ((*string <= '~') && (*string >= ' '))
    {
      if(x >= width && warp) {
        x = x0;
        y += size;
      }
      if (y >= height)break;
      drawChar_scr8(x, y, *string, size, pr, pg, pb, bgrd, br, bg, bb);
      x += size / 2;
      string++;
    }
    //  HAL_TIM_Base_Start_IT(&htim2);                  
 }

 void drawChar_scr8(int16_t x, int16_t y, uint8_t car, long size, 
                uint8_t pr, uint8_t pg, uint8_t pb, bool bgrd, uint8_t br, uint8_t bg, uint8_t bb){

  uint8_t temp, t1, t, pos, mode=1;
  uint16_t y0 = y;
  uint8_t csize = ((size/8) + ((size % 8) ? 1 : 0)) * (size/2);
  car = car - ' ';  // caracter value in ascii table - 32   

//  MAIN_LOGDEBUG1("csize : %d,  size%8 : %d \n", csize, ((size % 8) ? 1 : 0));

  if (size == 8){
      for(pos=0;pos<8;pos++){
          temp = asc2_0806[car][pos];//Call 0806 font, you need to take the model definition
          for(t=0;t<6;t++){                 
              if(temp&0x80){
  //                 Lcd_WriteData_16Bit(fc);
                   drawPixel_scr8(x+t,y+pos, pr, pg, pb );
              }
              else if (bgrd){
  //                Lcd_WriteData_16Bit(bc); 
                  drawPixel_scr8(x+t,y+pos, br, bg, bb );
              }
              else{
                drawPixel_scr8(x+t,y+pos, 0, 0, 0 );//Draw a point 
              }
              temp<<=1;
          }
       } 
  }
  else
  {
      for (t = 0; t < csize; t++)
      {
        if (size == 8)      temp = asc2_0806[car][t]; // not necessary 
        else if (size == 12)temp = asc2_1206[car][t];
        else if (size == 16)temp = asc2_1608[car][t];
        else if (size == 24)temp = asc2_2412[car][t];
        else return;
        for (t1 = 0; t1 < 8; t1++)
        {
          if (temp & 0x80) {
            drawPixel_scr8(x, y /*- 2*/, pr, pg, pb);
          }
          else if (bgrd){
            drawPixel_scr8(x, y /*- 2*/, br, bg, bb);
          }
          else{
             drawPixel_scr8(x, y /*- 2*/, 0, 0, 0);
          }
          temp <<= 1;
          y++;
          if (y >= (Scr_tab_width))return;
          if ((y - y0) == size)
          {
            y = y0;
            x++;
            if (x >= Scr_tab_width) return;
            break;
          }
        }
      }
  }
}

bool Matrix_scrolling_Text(uint8_t y0,uint8_t y1,uint8_t x0,uint8_t x1,char *string,uint16_t textcolor,uint16_t bg_color, long size,long Time, int Scrolldirection,int status) 
{
	uint16_t x=0,y=0,j,i,k=0,stringlength=0; uint8_t r=0; uint8_t g=0; uint8_t b=0;

	stringlength = strlen(string); 
	k=((SIZE_BOARD*5)/(size/2))*2;
  
  Serial.printf("\r\nString size: %u\r\n", stringlength);
  Serial.printf("K size: %u\r\n", k);
  Serial.printf("Size: %u\r\n", size);
  Serial.printf("\r\nFree Heap space: %u\r\n", ESP.getFreeHeap());
  

  //  tabpixels = (uint16_t*)malloc(sizeof(uint16_t[24][SIZE_BOARD*5])); 
   tabpixels = &u16TextBuffer[0]; 
    if (tabpixels == NULL)
    {
      Serial.println("\r\nSize tabpixels exepted");
      //if(scenario_read==true) ESP.restart();
      return 0;
    }
  int A=0,B=0,C=0,D=0;
  Serial.printf("After Free Heap space: %u\r\n", ESP.getFreeHeap());
  clearMatrix();
  set_Matrix_tab_fullsreem (bg_color);
  dump_text(0, 0, string, size,textcolor, bg_color, 1,Scrolldirection);
  
  if((status==1)&&(size==12))
     {A=13; B=11;C=13; D=25;}
  else if((status==1)&&(size==16))
     {A=11; B=11;C=13; D=25;}
  else if((status==1)&&(size==24))
     {A=16; B=11; C=16; D=32;}
  else if((status==2)&&(size==12))
     {A=-7; B=11;C=13; D=25;}
  else if((status==2)&&(size==16))
     {A=-4; B=11;C=13; D=25;}
  else if((status==2)&&(size==24))
     {A=0; B=11; C=16; D=32;}
  else if((status==3)&&(size==12))
     {A=-23; B=11;C=13; D=25;}
  else if((status==3)&&(size==16))
     {A=-20; B=11;C=13; D=25;}
  else if((status==3)&&(size==24))
     {A=-16; B=11; C=16; D=32;}

  Serial.printf("Size: %u\r\n", size);
 // Serial.printf("\r\n Les valeurs sont:\r\n A= %d\r\nB= %d\r\nC= %d\r\nD= %d. \r\n", A,B,C,D);  
  if(stringlength<=k)
  {
    if(Scrolldirection==scr_right)
    {
      for(uint16_t z=0; z<(stringlength*(size/2))+IMG_WIDTH;z++)
      {
        for(uint16_t y=C; y<D; y++)
        {
          for(uint16_t x=0; x<x1; x++)
          {
              DrawPixel(x,y-A,tabpixels[((y-B) * SIZE_BOARD*5) + x+z]);        
          }
          if(!scenario_read) break;
        }
        if(!scenario_read) break;
        delay(Time);
      }
    }

    if(Scrolldirection==scr_left)
    {
      for(uint16_t z=(stringlength*(size/2))+IMG_WIDTH; z>0; z--)
      {
        for(uint16_t y=0; y<y1; y++)
        {  
          for(uint16_t x=0; x<x1; x++)
          {
            if(!scenario_read) break;
            DrawPixel(x,y+y0,tabpixels[(y * SIZE_BOARD*5) + x+z]);
          }
          if(!scenario_read) break;
        }
        if(!scenario_read) break;
        delay(Time);
      }
    }
    // free(tabpixels);
    Serial.printf("Return After Free Heap space: %u\r\n", ESP.getFreeHeap());
    return 1;
  }
 else {Serial.println("\r\nSize exepted"); /*free(tabpixels);*/ return 0;}
}


uint8_t set_rotation (uint8_t  degres) 	
{
	rotation=degres;
}	


void DrawPixel(uint16_t row, uint16_t col, uint16_t value)
{
  uint16_t y_=col;
  uint16_t x_=row;

  if (rotation==degre_0)   {y_=col; x_=row;} ///////////////rotation 0°
  //if (rotation==degre_90)  {y_=IMG_HEIGHT-col-1;x_=row;} ///////////////rotation 90°
  if (rotation==degre_180) {y_=IMG_HEIGHT-col-1; x_=IMG_WIDTH-row-1;} 
  //if (rotation==degre_270) {y_=col; x_=IMG_WIDTH-row-1; }

  uint16_t z=(PANEL_RES_X*2)-1;
  uint16_t u=(PANEL_RES_X*2)-8;
  uint16_t x=z-x_;
  uint16_t y=(PANEL_RES_Y/2);
  uint16_t pannel=0;

  if((y_>=PANEL_RES_Y)&&(y_<PANEL_RES_Y*2)) u=IMG_WIDTH;
  else if((y_>=(PANEL_RES_Y*2))&&(y_<PANEL_RES_Y*3)) u=IMG_WIDTH*2;
  else if((y_>=(PANEL_RES_Y*3))&&(y_<PANEL_RES_Y*4)) u=IMG_WIDTH*3;
  else if((y_>=(PANEL_RES_Y*4))&&(y_<PANEL_RES_Y*5)) u=IMG_WIDTH*4;
  else u=0;
  x_+=u;

   if((y_<4)||((y_>=8)&&(y_<12))||((y_>=16)&&(y_<20))||((y_>=24)&&(y_<32))||((y_>=32)&&(y_<36))||((y_>=40)&&(y_<44))||((y_>=48)&&(y_<52))||((y_>=56)&&(y_<60)))
   {

    if(x_<8)
    {
      if(Panel_type==P8)
        x=z-x_-pannel; 
      if(Panel_type=P10)
        x=z-x_-pannel+1;
    }

    if((x_>=8)&&(x_<16))
    { 
      if(Panel_type==P8)
        x=(z-8)-x_-pannel;
      if(Panel_type=P10)
        x=(z-8)-x_-pannel+1;
    }

    if((x_>=16)&&(x_<24))
    { 
      if(Panel_type==P8)
       x=(z-16)-x_-pannel;
      if(Panel_type=P10)
       x=(z-16)-x_-pannel+1;
    }
    if((x_>=24)&&(x_<32))
    { 
      if(Panel_type==P8)
        x=(z-24)-x_-pannel;
      if(Panel_type=P10)
        x=(z-24)-x_-pannel+1;
    }

    if((x_>=32)&&(x_<40))
    { 
      if(Panel_type==P8)
        x=(z-32)-x_-pannel;
      if(Panel_type=P10)
        x=(z-32)-x_-pannel+1;
    }

    if((x_>=40)&&(x_<48))
    { 
      if(Panel_type==P8)
        x=(z-40)-x_-pannel;
      if(Panel_type=P10)
        x=(z-40)-x_-pannel+1;
    }

    if((x_>=48)&&(x_<56))
    { 
      if(Panel_type==P8)
        x=(z-48)-x_-pannel;
      if(Panel_type=P10)
        x=(z-48)-x_-pannel+1;
    }

    if((x_>=56)&&(x_<64))
    { 
      if(Panel_type==P8)
        x=(z-56)-x_-pannel;
      if(Panel_type=P10)
        x=(z-56)-x_-pannel+1;
    }


    if((x_>=64)&&(x_<72))
    { 
      if(Panel_type==P8)
      x=(z-64)-x_-pannel;
      if(Panel_type=P10)
      x=(z-64)-x_-pannel+1;
    }

    if((x_>=72)&&(x_<80))
    { 
      if(Panel_type==P8)
      x=(z-72)-x_-pannel;
      if(Panel_type=P10)
      x=(z-72)-x_-pannel+1;
    }

    if((x_>=80)&&(x_<88))
    { 
      if(Panel_type==P8)
      x=(z-80)-x_-pannel;
      if(Panel_type=P10)
      x=(z-80)-x_-pannel+1;
    }

    if((x_>=88)&&(x_<96))
    { 
      if(Panel_type==P8)
      x=(z-88)-x_-pannel;
      if(Panel_type=P10)
      x=(z-88)-x_-pannel+1;
    }

    if((x_>=96)&&(x_<104))
    { 
      if(Panel_type==P8)
      x=(z-96)-x_-pannel;
      if(Panel_type=P10)
      x=(z-96)-x_-pannel+1;
    }

    if((x_>=104)&&(x_<112))
    { 
      if(Panel_type==P8)
      x=(z-104)-x_-pannel;
      if(Panel_type=P10)
      x=(z-104)-x_-pannel+1;
    }

    if((x_>=112)&&(x_<120))
    { 
      if(Panel_type==P8)
      x=(z-112)-x_-pannel;
      if(Panel_type=P10)
      x=(z-112)-x_-pannel+1;
    }

    if((x_>=120)&&(x_<128))
    { 
      if(Panel_type==P8)
      x=(z-120)-x_-pannel;
      if(Panel_type=P10)
      x=(z-120)-x_-pannel+1;
    }

    if((x_>=128)&&(x_<136))
    { 
      if(Panel_type==P8)
      x=(z-128)-x_-pannel;
      if(Panel_type=P10)
      x=(z-128)-x_-pannel+1;
    }

    if((x_>=136)&&(x_<144))
    { 
      if(Panel_type==P8)
      x=(z-136)-x_-pannel;
      if(Panel_type=P10)
      x=(z-136)-x_-pannel+1;
    }

    if((x_>=144)&&(x_<152))
    { 
      if(Panel_type==P8)
      x=(z-144)-x_-pannel;
      if(Panel_type=P10)
      x=(z-144)-x_-pannel+1;
    }

    if((x_>=152)&&(x_<160))
    { 
      if(Panel_type==P8)
      x=(z-152)-x_-pannel;
      if(Panel_type=P10)
      x=(z-152)-x_-pannel+1;
    }

    if((x_>=160)&&(x_<168))
    { 
      if(Panel_type==P8)
      x=(z-160)-x_-pannel;
      if(Panel_type=P10)
      x=(z-160)-x_-pannel+1;
    }

    if((x_>=168)&&(x_<176))
    { 
      if(Panel_type==P8)
      x=(z-168)-x_-pannel;
      if(Panel_type=P10)
      x=(z-168)-x_-pannel+1;
    }

    if((x_>=176)&&(x_<184))
    { 
      if(Panel_type==P8)
      x=(z-176)-x_-pannel;
      if(Panel_type=P10)
      x=(z-176)-x_-pannel+1;
    }

    if((x_>=184)&&(x_<192))
    { 
      if(Panel_type==P8)
      x=(z-184)-x_-pannel;
      if(Panel_type=P10)
      x=(z-184)-x_-pannel+1;
    }

   }

   if(((y_>=4)&&(y_<8))||((y_>=12)&&(y_<16))||((y_>=28)&&(y_<32))||((y_>=20)&&(y_<24))||((y_>=36)&&(y_<40))||((y_>=44)&&(y_<48))||((y_>=52)&&(y_<56))||((y_>=60)&&(y_<64)))
   {

    if(x_<=7)
    { 
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-16)+x_+pannel; 
      if(Panel_type=P10)
         x=((PANEL_RES_X*2)-8)-x_+pannel;
    }
    if((x_>7)&&(x_<16))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-40)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-16)-x_+pannel;
    }
    if((x_>=16)&&(x_<24))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-64)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-24)-x_+pannel;
    }
    if((x_>=24)&&(x_<32))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-88)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-32)-x_+pannel;
    }
    if((x_>=32)&&(x_<40))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-112)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-40)-x_+pannel;
    }
    if((x_>=40)&&(x_<48))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-136)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-48)-x_+pannel;
    }
    if((x_>=48)&&(x_<56))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-160)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-56)-x_+pannel;
    }
    if((x_>=56)&&(x_<64))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-184)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-64)-x_+pannel;
    }
////////////////////////////////////16-20//////////////////////////////////////////////////////////////

////////////////////////////////////20-24//////////////////////////////////////////////////////////////

    if((x_>=64)&&(x_<72))
    {

      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-208)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-72)-x_+pannel;
    }
    if((x_>=72)&&(x_<80))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-232)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-80)-x_+pannel;
    }

    if((x_>=80)&&(x_<88))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-256)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-88)-x_+pannel;
    }

    if((x_>=88)&&(x_<96))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-280)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-96)-x_+pannel;
    }
      
    if((x_>=96)&&(x_<104))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-304)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-104)-x_+pannel;
    }

    if((x_>=104)&&(x_<112))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-328)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-112)-x_+pannel;
    }

    if((x_>=112)&&(x_<120))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-352)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-120)-x_+pannel;
    }

    if((x_>=120)&&(x_<128))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-376)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-128)-x_+pannel;
    }
////////////////////////////////////20-24//////////////////////////////////////////////////////////////

    if((x_>=128)&&(x_<136))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-400)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-136)-x_+pannel;
    }

    if((x_>=136)&&(x_<144))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-424)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-144)-x_+pannel;
    }

    if((x_>=144)&&(x_<152))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-448)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-152)-x_+pannel;
    }

    if((x_>=152)&&(x_<160))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-472)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-160)-x_+pannel;
    }

    if((x_>=160)&&(x_<168))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-496)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-168)-x_+pannel;
    }

    if((x_>=168)&&(x_<176))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-520)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-176)-x_+pannel;
    }

    if((x_>=176)&&(x_<184))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-544)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-184)-x_+pannel;
    }

    if((x_>=184)&&(x_<192))
    {
      if(Panel_type==P8)
        x=((PANEL_RES_X*2)-568)+x_+pannel;
      if(Panel_type==P10)
        x=((PANEL_RES_X*2)-192)-x_+pannel;
    }
   }

    if((y_==0)||(y_==4)||(y_==16)||(y_==20)||(y_==32)||(y_==36)||(y_==48)||(y_==52))
     y=7;
    if((y_==1)||(y_==5)||(y_==17)||(y_==21)||(y_==33)||(y_==37)||(y_==49)||(y_==53))
     y=6;
    if((y_==2)||(y_==6)||(y_==18)||(y_==22)||(y_==34)||(y_==38)||(y_==50)||(y_==54))
     y=5;
    if((y_==3)||(y_==7)||(y_==19)||(y_==23)||(y_==35)||(y_==39)||(y_==51)||(y_==55))
     y=4;
    if((y_==8)||(y_==12)||(y_==24)||(y_==28)||(y_==40)||(y_==44)||(y_==56)||(y_==60))
     y=3;
    if((y_==9)||(y_==13)||(y_==25)||(y_==29)||(y_==41)||(y_==45)||(y_==57)||(y_==61))
     y=2;
    if((y_==10)||(y_==14)||(y_==26)||(y_==30)||(y_==42)||(y_==46)||(y_==58)||(y_==62))
     y=1;
    if((y_==11)||(y_==15)||(y_==27)||(y_==31)||(y_==43)||(y_==47)||(y_==59)||(y_==63))
     y=0;
    dma_display->drawPixel(x,y,value);
}

void init_Matrix()
{
  Serial.println("...Starting Display");
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  //mxconfig.driver = HUB75_I2S_CFG::ICN2038S;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  virtualDisp = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y);
  dma_display->begin();
  dma_display->setBrightness8(Brightness); //0-255
  dma_display->fillScreenRGB888(128,0,0);
  delay(1000);
  dma_display->fillScreenRGB888(0,0,128);
  delay(1000);
  dma_display->clearScreen();  
  delay(1000);  
  Serial.println("**************** Starting Aurora Effects Demo ****************");
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  gif.begin(LITTLE_ENDIAN_PIXELS);
  dma_display->clearScreen();
}
