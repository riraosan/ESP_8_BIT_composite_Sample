
#pragma once

#include <M5Atom.h>
#include <AnimatedGIF.h>

#define LGFX_USE_V1
#include <LovyanGFX.h>
#include <lgfx_user/LGFX_ESP32_CVBS_sample.hpp>

#include <SPI.h>
#include <FS.h>
#include <SD.h>

static LGFX_CVBS   _cvbs;
static LGFX_Sprite _sprite(&_cvbs);

class Video {
public:
  Video() : _filename(""),
            _isActive(false),
            _isOpen(false) {}

  void begin(void) {
    _display_width  = _cvbs.width() - 1;
    _display_height = _cvbs.height() - 1;

    log_i("width, %d, height, %d", _display_width, _display_height);

    _sprite.setColorDepth(8);
    _sprite.setRotation(0);
    if(!_sprite.createSprite(241, 203)){
      log_e("can not allocate sprite buffer.");
    }

    _cvbs.begin();
    _cvbs.fillScreen(0);

    _gif.begin(LITTLE_ENDIAN_PIXELS);
    log_i("start CVBS");
  }

  void update(void) {
    if (_isActive) {
      long lTimeStart = lgfx::v1::millis();
      int  waitTime;

      if (_gif.playFrame(false, &waitTime)) {
        long delta = lgfx::v1::millis() - lTimeStart;
        _sprite.pushRotateZoom(0, 0.9, 0.9);
        if (waitTime > delta) {
          delay(waitTime - delta);
        } else {
          // log_i("No. %04d waitTime %d delta %d", frameCount, waitTime, delta);
        }
      } else {
        // stop();
        closeGif();
        openGif();
      }
    }
  }

  void setSd(SDFS *sd) {
    _pSD = sd;
  }

  void setFilename(String name) {
    _filename = name;
  }

  void openGif(void) {
    if (_gif.open(_filename.c_str(), _GIFOpenFile, _GIFCloseFile, _GIFReadFile, _GIFSeekFile, _GIFDraw)) {
      _isOpen = true;
    }
  }

  void closeGif(void) {
    if (_isOpen) {
      _gif.close();
      _isOpen = false;
      // _isActive = false;
      // _cvbs.fillScreen(0);
    }
  }

  void resetGif(void) {
    if (_isOpen) {
      _gif.reset();
    }
  }

  void start(void) {
    if (_isOpen)
      _isActive = true;
  }

  void stop(void) {
    if (_isOpen)
      _isActive = false;
  }

  bool state(void) {
    return _isActive;
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
        }              // while looking for opaque pixels
        if (iCount) {  // any opaque pixels?
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
    }
    // log_i("pDraw->iX, %d, y, %d", pDraw->iX, y);
    _sprite.setWindow(pDraw->iX, y, iWidth, 1);
    _sprite.pushPixels(usTemp, iWidth, true);
  }

  static SDFS *_pSD;
  static File  _gifFile;

  static int _display_width;
  static int _display_height;

  AnimatedGIF _gif;
  String      _filename;

  bool _isActive;
  bool _isOpen;
};

SDFS *Video::_pSD = nullptr;
File  Video::_gifFile;

int Video::_display_width  = 0;
int Video::_display_height = 0;
