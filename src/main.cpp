
#include <AnimatedGIF.h>
#include <Arduino.h>
#include <ESP_8_BIT_GFX.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <Wire.h>

constexpr uint16_t kIrLed = 12;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

// Create an instance of the graphics library
ESP_8_BIT_GFX videoOut(true, 16);
AnimatedGIF gif;
File _file;

// Vertical margin to compensate for aspect ratio
constexpr int _gifOffset_x  = 6;
constexpr int _gifOffset_y  = 50;
constexpr int _textOffset_x = 6;
constexpr int _textOffset_y = 6;

void *_GIFOpenFile(const char *fname, int32_t *pSize) {
  _file = SPIFFS.open(fname);

  if (_file) {
    *pSize = _file.size();
    return (void *)&_file;
  }

  return NULL;
}

void _GIFCloseFile(void *pHandle) {
  File *f = static_cast<File *>(pHandle);

  if (f != NULL)
    f->close();
}

int32_t _GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f    = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1;  // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;

  iBytesRead  = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();

  return iBytesRead;
}

int32_t _GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  int i   = micros();
  File *f = static_cast<File *>(pFile->fHandle);

  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i           = micros() - i;
  //  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
}

void _GIFDraw(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y;

  usPalette = pDraw->pPalette;
  y         = pDraw->iY + pDraw->y;  // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2)  // restore to background color
  {
    for (x = 0; x < pDraw->iWidth; x++) {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency)  // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd   = s + pDraw->iWidth;
    x      = 0;
    iCount = 0;  // count non-transparent pixels
    while (x < pDraw->iWidth) {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)  // done, stop
        {
          s--;    // back up to treat it like transparent
        } else {  // opaque
          *d++ = usPalette[c];
          iCount++;
        }
      }            // while looking for opaque pixels
      if (iCount)  // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++) {
          videoOut.drawPixel(pDraw->iX + x + xOffset + _gifOffset_x, y + _gifOffset_y, usTemp[xOffset]);
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount) {
        x += iCount;  // skip these
        iCount = 0;
      }
    }
  } else {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++) {
      videoOut.drawPixel(x + _gifOffset_x, y + _gifOffset_y, usPalette[*s++]);
    }
  }
}

void setup() {
  irsend.begin();
  delay(1000);
  irsend.sendPanasonic(0x555A, 0xF148688B);
  delay(10);
  irsend.sendPanasonic(0x555A, 0xF148688B);
  delay(2000);

  log_d("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  if (!SPIFFS.begin()) {
    log_e("FILESYSTEM Mount Failed");
    return;
  }
  videoOut.begin();
  videoOut.copyAfterSwap = true;  // gif library depends on data from previous buffer
  videoOut.fillScreen(0);
  videoOut.waitForFrame();

  gif.begin(LITTLE_ENDIAN_PIXELS);
}

void loop() {
  if (gif.open("/drain.gif", _GIFOpenFile, _GIFCloseFile, _GIFReadFile, _GIFSeekFile, _GIFDraw)) {
    while (gif.playFrame(true, NULL)) {
      videoOut.setTextSize(1);
      videoOut.setTextColor(0xFFFF, 0x0000);
      videoOut.printEfont("          By using           ", _textOffset_x, _textOffset_y + 16 * 10);
      videoOut.printEfont("ESP_8_BIT_composite Library  ", _textOffset_x, _textOffset_y + 16 * 11);
      videoOut.printEfont("AnimatedGIF Library          ", _textOffset_x, _textOffset_y + 16 * 12);
      videoOut.printEfont("EfontWrapper Library         ", _textOffset_x, _textOffset_y + 16 * 13);

#if defined(DEBUG)
      // x:0~28 y:0~13
      videoOut.setTextColor(0xfa20, 0x0000);
      videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 0);
      videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 0);
      videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 13);
      videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 13);
#endif

      videoOut.waitForFrame();
    }
    videoOut.waitForFrame();
    gif.close();
  }
}
