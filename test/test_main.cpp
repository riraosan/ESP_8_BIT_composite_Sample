#include <unity.h>
#include <Arduino.h>

// #define BUILD_TEST            // OK
// #define RGB_TEST              // OK
// #define PDQ_TEST              // OK
// #define PARTY_PARROT          // OK
// #define CLOCK_SAMPLE          // OK
// #define FLASH_MEM_SPRITE      // OK
// #define COLLISION_CIRCLES     // NG
// #define MOVING_CIRCLES        // OK
// #define MOVING_ICONS          // NG なぜか真ん中に境目が見えるようになった。
// #define ROTATED_ZOOMS_SAMPLE  // OK
#define LONG_TEXT_SCROLL  // NG

#if !defined LGFX_USE_V1
#define LGFX_USE_V1
#endif

#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_ESP32_CVBS_sample.hpp>

#if defined(BUILD_TEST)

#include "buildtest.h"  // OK

#elif defined(RGB_TEST)  // OK

static LGFX_CVBS   _cvbs;
static LGFX_Sprite _sprite(&_cvbs);

void setup(void) {
  _cvbs.init();

  _cvbs.fillScreen(0x0000);  //黒
  delay(1000);
  _cvbs.fillScreen(0xF800);  //赤
  delay(1000);
  _cvbs.fillScreen(0x07E0);  //緑
  delay(1000);
  _cvbs.fillScreen(0x001F);  //青
  delay(1000);
  _cvbs.fillScreen(0xffff);  //白
  delay(1000);
  _cvbs.fillScreen(0x0000);  //黒
  delay(1000);
}

void loop(void) {
  _cvbs.waitForFrame();
}

#elif defined(PDQ_TEST)

using TFT_eSPI = LGFX_CVBS;

#include "./TFT_graphicstest_PDQ.ino"

uint32_t _wait = 500;

void begin(void) {
  tft.setCopyAfterSwap(true);
  tft.setColorDepth(8);
  tft.setRotation(0);

  tft.begin();
}

void _testFillScreen(void) {
  uint32_t usecFillScreen = testFillScreen();
  Serial.print(F("Screen fill              "));
  Serial.println(usecFillScreen);
  delay(_wait);
}

void _testText(void) {
  uint32_t usecText = testText();
  Serial.print(F("Text                     "));
  Serial.println(usecText);
  delay(_wait);
}

void _testPixels(void) {
  uint32_t usecPixels = testPixels();
  Serial.print(F("Pixels                   "));
  Serial.println(usecPixels);
  delay(_wait);
}

void _testLines(void) {
  uint32_t usecLines = testLines(TFT_BLUE);
  Serial.print(F("Lines                    "));
  Serial.println(usecLines);
  delay(_wait);
}

void _testFastLines(void) {
  uint32_t usecFastLines = testFastLines(TFT_RED, TFT_BLUE);
  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(usecFastLines);
  delay(_wait);
}

void _testRects(void) {
  uint32_t usecRects = testRects(TFT_GREEN);
  Serial.print(F("Rectangles (outline)     "));
  Serial.println(usecRects);
  delay(_wait);
}

void _testFilledRects(void) {
  uint32_t usecFilledRects = testFilledRects(TFT_YELLOW, TFT_MAGENTA);
  Serial.print(F("Rectangles (filled)      "));
  Serial.println(usecFilledRects);
  delay(_wait);
}

void _testFilledCircles(void) {
  uint32_t usecFilledCircles = testFilledCircles(10, TFT_MAGENTA);
  Serial.print(F("Circles (filled)         "));
  Serial.println(usecFilledCircles);
  delay(_wait);
}

void _testCircles(void) {
  uint32_t usecCircles = testCircles(10, TFT_WHITE);
  Serial.print(F("Circles (outline)        "));
  Serial.println(usecCircles);
  delay(_wait);
}

void _testTriangles(void) {
  uint32_t usecTriangles = testTriangles();
  Serial.print(F("Triangles (outline)      "));
  Serial.println(usecTriangles);
  delay(_wait);
}

void _testFilledTriangles(void) {
  uint32_t usecFilledTrangles = testFilledTriangles();
  Serial.print(F("Triangles (filled)       "));
  Serial.println(usecFilledTrangles);
  delay(_wait);
}

void _testRoundRects(void) {
  uint32_t usecRoundRects = testRoundRects();
  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(usecRoundRects);
  delay(_wait);
}

void _testFilledRoundRects(void) {
  uint32_t usedFilledRoundRects = testFilledRoundRects();
  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(usedFilledRoundRects);
  delay(_wait);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(begin);
  RUN_TEST(_testFillScreen);
  RUN_TEST(_testText);
  RUN_TEST(_testPixels);
  RUN_TEST(_testLines);
  RUN_TEST(_testFastLines);
  RUN_TEST(_testRects);
  RUN_TEST(_testFilledRects);
  RUN_TEST(_testFilledCircles);
  RUN_TEST(_testCircles);
  RUN_TEST(_testTriangles);
  RUN_TEST(_testFilledTriangles);
  RUN_TEST(_testRoundRects);
  RUN_TEST(_testFilledRoundRects);

  Serial.println(F("Done!"));

  uint16_t c = 4;
  int8_t   d = 1;
  for (int32_t i = 0; i < tft.height(); i++) {
    tft.drawFastHLine(0, i, tft.width(), c);
    c += d;
    if (c <= 4 || c >= 11)
      d = -d;
  }

  tft.setCursor(10, 10);
  tft.setTextColor(TFT_MAGENTA);
  tft.setTextSize(2);

  tft.println(F("  LovyanGFX test"));

  delay(_wait);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.println(F(""));
  tft.setTextSize(1);
  tft.println(F(""));
  tft.setTextColor(tft.color332(0x80, 0x80, 0x80));

  tft.println(F(""));

  tft.setTextColor(TFT_GREEN);
  tft.println(F(" Benchmark               microseconds"));
  tft.println(F(""));
  tft.setTextColor(TFT_YELLOW);

  delay(_wait);

  tft.setTextSize(1);
  tft.println(F(""));
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.print(F(" Benchmark Complete!"));

  delay(60 * 1000L);

  UNITY_END();
}

void loop() {
}

#elif defined(PARTY_PARROT)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/PartyParrot/PartyParrot.ino"

#elif defined(CLOCK_SAMPLE)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/ClockSample/ClockSample.ino"

#elif defined(FLASH_MEM_SPRITE)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/FlashMemSprite/FlashMemSprite.ino"

#elif defined(COLLISION_CIRCLES)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/CollisionCircles/CollisionCircles.ino"

#elif defined(MOVING_CIRCLES)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/MovingCircles/MovingCircles.ino"

#elif defined(MOVING_ICONS)

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/MovingIcons/MovingIcons.ino"

#elif defined(ROTATED_ZOOMS_SAMPLE)  //  width = 180 OK, 183 NG

using LGFX = LGFX_CVBS;
#include "../examples/Sprite/RotatedZoomSample/RotatedZoomSample.ino"

#elif defined(LONG_TEXT_SCROLL)

using LGFX = LGFX_CVBS;
#include "../examples/Standard/LongTextScroll/LongTextScroll.ino"

#endif
