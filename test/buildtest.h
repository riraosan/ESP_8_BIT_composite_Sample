#if !defined LGFX_AUTODETECT
#define LGFX_AUTODETECT
#endif

#if !defined LGFX_USE_V1
#define LGFX_USE_V1
#endif

#if defined(ARDUINO) && defined(ESP32)
#include <SPIFFS.h>
#endif

#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_ESP32_CVBS.hpp>

static LGFX_CVBS   _lgfx;
static LGFX_Sprite _sprite(&_lgfx);

void build_setup(void) {
  _lgfx.init();
  _lgfx.setRotation(1);
  _lgfx.setBrightness(128);
  _lgfx.setColorDepth(24);
  _lgfx.drawPixel(0, 0, 0xFFFF);
  _lgfx.drawFastVLine(2, 0, 100, _lgfx.color888(255, 0, 0));
  _lgfx.drawFastVLine(4, 0, 100, _lgfx.color565(0, 255, 0));
  _lgfx.drawFastVLine(6, 0, 100, _lgfx.color332(0, 0, 255));
  uint32_t red = 0xFF0000;
  _lgfx.drawFastHLine(0, 2, 100, red);
  _lgfx.drawFastHLine(0, 4, 100, 0x00FF00U);
  _lgfx.drawFastHLine(0, 6, 100, (uint32_t)0xFF);
  uint16_t green = 0x07E0;
  _lgfx.drawRect(10, 10, 50, 50, 0xF800);
  _lgfx.drawRect(12, 12, 50, 50, green);
  _lgfx.drawRect(14, 14, 50, 50, (uint16_t)0x1F);
  uint8_t blue = 0x03;
  _lgfx.fillRect(20, 20, 20, 20, (uint8_t)0xE0);
  _lgfx.fillRect(30, 30, 20, 20, (uint8_t)0x1C);
  _lgfx.fillRect(40, 40, 20, 20, blue);
  _lgfx.setColor(0xFF0000U);
  _lgfx.fillCircle(40, 80, 20);
  _lgfx.fillEllipse(80, 40, 10, 20);
  _lgfx.fillArc(80, 80, 20, 10, 0, 90);
  _lgfx.fillTriangle(80, 80, 60, 80, 80, 60);
  _lgfx.setColor(0x0000FFU);
  _lgfx.drawCircle(40, 80, 20);
  _lgfx.drawEllipse(80, 40, 10, 20);
  _lgfx.drawArc(80, 80, 20, 10, 0, 90);
  _lgfx.drawTriangle(60, 80, 80, 80, 80, 60);
  _lgfx.setColor(0x00FF00U);
  _lgfx.drawBezier(60, 80, 80, 80, 80, 60);
  _lgfx.drawBezier(60, 80, 80, 20, 20, 80, 80, 60);
  _lgfx.drawGradientLine(0, 80, 80, 0, 0xFF0000U, 0x0000FFU);

  _lgfx.fillScreen(0xFFFFFFu);
  _lgfx.setColor(0x00FF00u);
  _lgfx.fillScreen();
  _lgfx.clear(0xFFFFFFu);
  _lgfx.setBaseColor(0x000000u);
  _lgfx.clear();
  _lgfx.drawLine(0, 1, 39, 40, red);
  _lgfx.drawLine(1, 0, 40, 39, blue);
  _lgfx.startWrite();
  _lgfx.drawLine(38, 0, 0, 38, 0xFFFF00U);
  _lgfx.drawLine(39, 1, 1, 39, 0xFF00FFU);
  _lgfx.drawLine(40, 2, 2, 40, 0x00FFFFU);
  _lgfx.endWrite();
  _lgfx.startWrite();
  _lgfx.startWrite();
  _lgfx.startWrite();
  _lgfx.endWrite();
  _lgfx.endWrite();
  _lgfx.endWrite();
  _lgfx.endWrite();
  _lgfx.startWrite();
  _lgfx.startWrite();
  _lgfx.drawPixel(0, 0);
  _lgfx.endTransaction();
  _lgfx.beginTransaction();
  _lgfx.drawPixel(0, 0);
  _lgfx.endWrite();
  _lgfx.endWrite();
  _lgfx.startWrite();
  for (uint32_t x = 0; x < 128; ++x) {
    for (uint32_t y = 0; y < 128; ++y) {
      _lgfx.writePixel(x, y, _lgfx.color888(x * 2, x + y, y * 2));
    }
  }
  _lgfx.endWrite();

  _sprite.setColorDepth(24);
  _sprite.createSprite(65, 65);

  for (uint32_t x = 0; x < 64; ++x) {
    for (uint32_t y = 0; y < 64; ++y) {
      _sprite.drawPixel(x, y, _lgfx.color888(3 + x * 4, (x + y) * 2, 3 + y * 4));
    }
  }
  _sprite.drawRect(0, 0, 65, 65, 0xFFFF);
  _sprite.pushSprite(64, 0);
  _sprite.pushSprite(&_lgfx, 0, 64);

  _sprite.setPivot(32, 32);
  int32_t center_x = _lgfx.width() / 2;
  int32_t center_y = _lgfx.height() / 2;
  _lgfx.startWrite();
  for (int angle = 0; angle <= 360; ++angle) {
    _sprite.pushRotateZoom(center_x, center_y, angle, 2.5, 3);
    if ((angle % 36) == 0) _lgfx.display();
  }
  _lgfx.endWrite();

  _sprite.deleteSprite();

  _sprite.setColorDepth(4);
  _sprite.createSprite(65, 65);
  _sprite.setPaletteColor(1, 0x0000FFU);
  _sprite.setPaletteColor(2, 0x00FF00U);
  _sprite.setPaletteColor(3, 0xFF0000U);

  _sprite.fillRect(10, 10, 45, 45, 1);
  _sprite.fillCircle(32, 32, 22, 2);
  _sprite.fillTriangle(32, 12, 15, 43, 49, 43, 3);

  _sprite.pushSprite(0, 0, 0);

  _lgfx.startWrite();

#if defined(ARDUINO) && defined(ESP32)
  _lgfx.drawBmpFile(SPIFFS, "/test.bmp");
  _lgfx.drawPngFile(SPIFFS, "/test.png");
  _lgfx.drawJpgFile(SPIFFS, "/test.jpg");
#endif
}

void build_loop(void) {
  static int   count = 0;
  static int   a     = 0;
  static int   x     = 0;
  static int   y     = 0;
  static float zoom  = 3;
  ++count;
  if ((a += 1) >= 360) a -= 360;
  if ((x += 2) >= _lgfx.width()) x -= _lgfx.width();
  if ((y += 1) >= _lgfx.height()) y -= _lgfx.height();
  _sprite.setPaletteColor(3, _lgfx.color888(count & 0xFF, 0, 0));
  _sprite.pushRotateZoom(x, y, a, zoom, zoom, 0);
  if ((count % 100) == 0) _lgfx.display();  // 電子ペーパーの場合の表示更新を 100回に一度行う
}
