
#pragma once

#include <M5Atom.h>
#include <AnimatedGIF.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_ESP32_CVBS.hpp>

#include <SPI.h>
#include <FS.h>
#include <SD.h>

static LGFX_CVBS   _lgfx;
static LGFX_Sprite _splite(&_lgfx);

constexpr int _gifOffset_x  = 30;
constexpr int _gifOffset_y  = 55;
constexpr int _textOffset_x = 6;
constexpr int _textOffset_y = 6;

class Video {
public:
  Video() : _filename("") {
  }

  void begin(void) {
    _display_width  = _lgfx.width();
    _display_height = _lgfx.height();

    _lgfx.begin();

    _lgfx.fillScreen(0x00);

    _gif.begin(LITTLE_ENDIAN_PIXELS);
    log_i("start CVBS");
  }

  void update(void) {
    int  frameCount = 0;
    long lTimeStart = 0;
    long startTime  = lgfx::v1::millis();

    if (_gif.open(_filename.c_str(), _GIFOpenFile, _GIFCloseFile, _GIFReadFile, _GIFSeekFile, _GIFDraw)) {
      log_printf("start gif animation\n");
      lTimeStart = lgfx::v1::millis();
      int waitTime;

      while (_gif.playFrame(false, &waitTime)) {
        _lgfx.waitForFrame();
        long lTimeEnd = lgfx::v1::millis();

        long delta = lTimeEnd - lTimeStart;
        if (waitTime >= delta) {
          delay(waitTime - delta);
        } else {
          log_i("No. %04d waitTime %d delta %d", frameCount, waitTime, delta);
        }

        frameCount++;

        lTimeStart = lgfx::v1::millis();
      }

      long playTime = lgfx::v1::millis() - startTime;
      _lgfx.waitForFrame();
      log_printf("end gif animation: time = %d\n", playTime);
      _gif.close();
    }
  }

  void setSd(SDFS *sd) {
    _pSD = sd;
  }

  void setFilename(String name) {
    _filename = name;
  }

private:
  static void *_GIFOpenFile(const char *fname, int32_t *pSize) {
    _gifFile = _pSD->open(fname);

    if (_gifFile) {
      *pSize = _gifFile.size();
      return (void *)&_gifFile;
    }

    return nullptr;
  }

  static void _GIFCloseFile(void *pHandle) {
    File *f = static_cast<File *>(pHandle);

    if (f != nullptr)
      f->close();
  }

  static int32_t _GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
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

  static int32_t _GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
    int   i = micros();
    File *f = static_cast<File *>(pFile->fHandle);

    f->seek(iPosition);
    pFile->iPos = (int32_t)f->position();
    i           = micros() - i;
    // log_i("Seek time = %d us\n", i);
    return pFile->iPos;
  }

#if true
  static void _GIFDraw(GIFDRAW *pDraw) {
    uint8_t  *s;
    uint16_t *d, *usPalette, usTemp[320];
    int       x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > _display_width)
      iWidth = _display_width - pDraw->iX;

    usPalette = pDraw->pPalette;
    y         = pDraw->iY + pDraw->y;  // current line

    if (y >= _display_height || pDraw->iX >= _display_width || iWidth < 1)
      return;

    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2)  // restore to background color
    {
      for (x = 0; x < iWidth; x++) {
        if (s[x] == pDraw->ucTransparent)
          s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }

    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency)  // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int      x, iCount;
      pEnd   = s + iWidth;
      x      = 0;
      iCount = 0;  // count non-transparent pixels
      while (x < iWidth) {
        c = ucTransparent - 1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent)  // done, stop
          {
            s--;  // back up to treat it like transparent
          } else  // opaque
          {
            *d++ = usPalette[c];
            iCount++;
          }
        }            // while looking for opaque pixels
        if (iCount)  // any opaque pixels?
        {
          _lgfx.setWindow(_gifOffset_x, y + _gifOffset_y, iWidth, 1);
          _lgfx.writePixels(usTemp, iCount, true);
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
      for (x = 0; x < iWidth; x++)
        usTemp[x] = usPalette[*s++];

      _lgfx.setWindow(_gifOffset_x, y + _gifOffset_y, iWidth, 1);
      _lgfx.writePixels(usTemp, iWidth, true);
    }
  }
#else
  static void _GIFDraw(GIFDRAW *pDraw) {
    uint8_t  *s;
    uint16_t *d, *usPalette, usTemp[320];
    int       x, y;

    usPalette = pDraw->pPalette;
    y         = pDraw->iY + pDraw->y;  // current line

    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) {  // restore to background color
      for (x = 0; x < pDraw->iWidth; x++) {
        if (s[x] == pDraw->ucTransparent)
          s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) {  // if transparency used
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int      x, iCount;
      pEnd   = s + pDraw->iWidth;
      x      = 0;
      iCount = 0;  // count non-transparent pixels
      while (x < pDraw->iWidth) {
        c = ucTransparent - 1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent) {  // done, stop
            s--;                     // back up to treat it like transparent
          } else {                   // opaque
            *d++ = usPalette[c];
            iCount++;
          }
        }              // while looking for opaque pixels
        if (iCount) {  // any opaque pixels?
          for (int xOffset = 0; xOffset < iCount; xOffset++) {
            _lgfx.drawPixel(pDraw->iX + x + xOffset + _gifOffset_x, y + _gifOffset_y, usTemp[xOffset]);
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
        _lgfx.drawPixel(x + _gifOffset_x, y + _gifOffset_y, usPalette[*s++]);
      }
    }
  }
#endif

  static SDFS *_pSD;
  static File  _gifFile;

  static int _display_width;
  static int _display_height;

  AnimatedGIF _gif;
  String      _filename;
};

SDFS *Video::_pSD = nullptr;
File  Video::_gifFile;

int Video::_display_width  = 0;
int Video::_display_height = 0;
