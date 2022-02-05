
#include <AnimatedGIF.h>
#include <Arduino.h>
#include <ESP_8_BIT_GFX.h>
#include <FS.h>
#include <M5Atom.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

// Create an instance of the graphics library
ESP_8_BIT_GFX videoOut(true, 16);
AnimatedGIF gif;
File _file;

// Vertical margin to compensate for aspect ratio
constexpr int _gifOffset_x  = 30;
constexpr int _gifOffset_y  = 50;
constexpr int _textOffset_x = 6;
constexpr int _textOffset_y = 6;

constexpr char NON_GIF[] = "/non.gif";

inline void *_GIFOpenFile(const char *fname, int32_t *pSize) {
  _file = SD.open(fname);

  if (_file) {
    *pSize = _file.size();
    return (void *)&_file;
  }

  return nullptr;
}

inline void _GIFCloseFile(void *pHandle) {
  File *f = static_cast<File *>(pHandle);

  if (f != nullptr)
    f->close();
}

inline int32_t _GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
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

inline int32_t _GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  int i   = micros();
  File *f = static_cast<File *>(pFile->fHandle);

  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i           = micros() - i;
  //  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
}

inline void _GIFDraw(GIFDRAW *pDraw) {
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
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  M5.begin(true, false, false);
  SPI.begin(23, 33, 19, -1);

  SD.end();

  delay(1000);

  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  }

  if (SD.cardType() == CARD_NONE) {
    log_e("No SD card attached");
    return;
  }

  log_i("SD card attached");

  videoOut.begin();
  videoOut.copyAfterSwap = true;  // gif library depends on data from previous buffer
  videoOut.fillScreen(0);
  videoOut.waitForFrame();

  gif.begin(LITTLE_ENDIAN_PIXELS);

  log_i("start videoOut");

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void loop() {
  int waitTime    = 0;
  int frameCount  = 0;
  long lTimeStart = 0;
  long lTimeEnd   = 0;

  if (gif.open(NON_GIF, _GIFOpenFile, _GIFCloseFile, _GIFReadFile, _GIFSeekFile, _GIFDraw)) {
    log_n("start gif animation");
    lTimeStart = millis();

    while (gif.playFrame(false, &waitTime)) {
      videoOut.waitForFrame();
      lTimeEnd = millis();

      long delta = lTimeEnd - lTimeStart;
      if (waitTime > delta) {
        delay(waitTime - delta);
      } else {
        // log_w("waitTime[%d] < delta[%d]...", waitTime, delta);
      }

      // log_d("frame No.[%04d], wait %04d[ms], wait for Frame %04d[ms]", frameCount, waitTime, delta);
      frameCount++;

#if defined(DEBUG)
      // x:0~28 y:0~13
      videoOut.setTextColor(0xfa20, 0x0000);
      videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 0);
      videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 0);
      videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 13);
      videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 13);
#endif
      lTimeStart = millis();
    }
    videoOut.waitForFrame();
    log_n("end gif animation");
    gif.close();
  }
}
