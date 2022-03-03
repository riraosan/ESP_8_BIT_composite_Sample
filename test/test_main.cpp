#include <unity.h>
#include <Arduino.h>

//#define BUILD_TEST
//#define RGB_TEST
#define PDQ_TEST

#if defined(BUILD_TEST)

#include "buildtest.h"  // OK

#elif defined(RGB_TEST)  // OK

#if !defined LGFX_USE_V1
#define LGFX_USE_V1
#endif

#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_ESP32_CVBS.hpp>

static LGFX_CVBS   _cvbs;
static LGFX_Sprite _sprite(&_cvbs);

void begin(void) {
  _cvbs.setCopyAfterSwap(true);
  _cvbs.setColorDepth(8);
  _cvbs.setRotation(0);

  _cvbs.begin();

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

void update(void) {
  _cvbs.waitForFrame();
}

#elif defined(PDQ_TEST)
#include "test_TFT_graphicstest_PDQ.h"

//OK
void _testPixels(void) {
  uint32_t usecPixels = testPixels();
  Serial.print(F("Pixels                   "));
  Serial.println(usecPixels);
  delay(2000);
}

//OK
void _testLines(void) {
  uint32_t usecLines = testLines(_cvbs.color16to8(TFT_BLUE));
  Serial.print(F("Lines                    "));
  Serial.println(usecLines);
  delay(100);
}

#endif

void setUp(void) {
}

void tearDown(void) {
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(begin);
  RUN_TEST(_testPixels);
  RUN_TEST(_testLines);
}

void loop() {
  _cvbs.waitForFrame();
  UNITY_END();
}
