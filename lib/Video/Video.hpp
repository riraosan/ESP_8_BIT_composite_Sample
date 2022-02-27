
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

constexpr int _gifOffset_x  = 6;
constexpr int _gifOffset_y  = 45;
constexpr int _textOffset_x = 6;
constexpr int _textOffset_y = 6;

#define BUILD_TEST
class Video {
public:
  Video() : _filename("") {
  }

  void begin(void) {
    _lgfx.setColorDepth(8);
    _lgfx.setRotation(0);
    _lgfx.begin();

    // TODO このダサいところをなんとかしたい
    _lgfx.fillScreen(0x00);
    _lgfx.waitForFrame();
    _lgfx.fillScreen(0x00);
    _lgfx.waitForFrame();

    _gif.begin(LITTLE_ENDIAN_PIXELS);
    log_i("start CVBS");
  }

#if 1
  void update(void) {
    _lgfx.waitForFrame();
    int  frameCount = 0;
    long lTimeStart = 0;

    if (_gif.open(_filename.c_str(), _GIFOpenFile, _GIFCloseFile, _GIFReadFile, _GIFSeekFile, _GIFDraw)) {
      log_i("start gif animation");
      lTimeStart = millis();
      int waitTime;

      while (_gif.playFrame(false, &waitTime)) {
        _lgfx.waitForFrame();
        long lTimeEnd = millis();

        long delta = lTimeEnd - lTimeStart;
        if (waitTime >= delta) {
          delay(waitTime - delta);
        } else {
          // log_w("Frame No.[%04d], waitTime[%d] < delta[%d]...", frameCount, waitTime, delta);
        }

        frameCount++;

#if defined(DEBUG)
        // x:0~28 y:0~13
        _videoOut.setTextColor(0xfa20, 0x0000);
        _videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 0);
        _videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 0);
        _videoOut.printEfont("*", _textOffset_x + 8 * 0, _textOffset_y + 16 * 13);
        _videoOut.printEfont("*", _textOffset_x + 8 * 28, _textOffset_y + 16 * 13);
#endif
        lTimeStart = millis();
      }

      _lgfx.waitForFrame();
      log_i("end gif animation");
      _gif.close();
    }
  }
#else
  void update(void) {
    _lgfx.fillScreen(0x00);
    _lgfx.waitForFrame();
    delay(1000);
    _lgfx.fillScreen(0xF800);  //赤
    _lgfx.waitForFrame();
    delay(1000);
    _lgfx.fillScreen(0x07E0);  //緑
    _lgfx.waitForFrame();
    delay(1000);
    _lgfx.fillScreen(0x001F);  //青
    _lgfx.waitForFrame();
    delay(1000);
    _lgfx.fillScreen(0xffff);  //白
    _lgfx.waitForFrame();
    delay(1000);
  }
#endif

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

  static SDFS *_pSD;
  static File  _gifFile;

  AnimatedGIF _gif;
  String      _filename;
};

SDFS *Video::_pSD = nullptr;
File  Video::_gifFile;
