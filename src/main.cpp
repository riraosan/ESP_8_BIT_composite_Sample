
#include <M5Atom.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.hpp>
#include <Button2.h>

Audio   _mp3Audio;
Button2 _button;

void clickHandler(Button2& btn) {
  switch (btn.getClickType()) {
    case SINGLE_CLICK:
      log_i("single ");

      Serial.print("start\n");  // to video
      delay(780);
      _mp3Audio.start();
      M5.dis.drawpix(0, 0x00FF00);
      break;
    case DOUBLE_CLICK:
      log_i("double ");
      break;
    case TRIPLE_CLICK:
      log_i("triple ");

      Serial.print("cue\n");  // to video
      _mp3Audio.stop();
      M5.dis.drawpix(0, 0xFF0000);
      break;
    case LONG_CLICK:
      log_i("long");

      _mp3Audio.stop();
      M5.dis.drawpix(0, 0x000000);
      _mp3Audio.disconnect();
      Serial.print("restart\n");  // to video
      delay(2000);
      ESP.restart();
      break;
  }
  log_i("click (%d)", btn.getNumberOfClicks());
}

void initButton(void) {
  _button.setClickHandler(clickHandler);
  _button.setLongClickHandler(clickHandler);
  _button.setDoubleClickHandler(clickHandler);
  _button.setTripleClickHandler(clickHandler);
  _button.begin(39);  // for ATOM Lite
}

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  Serial.begin(115200);

  M5.begin(false, false, true);  // using LED
  M5.dis.drawpix(0, 0xFF0000);

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 24000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _mp3Audio.setAutoReconnect(false);  // do not connect automatically.
    _mp3Audio.setResetBle(true);        // do reset ble before starting ble.
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    _mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setSDFS(&SD);
    _mp3Audio.begin();
  }

  M5.dis.drawpix(0, 0xFFFFFF);
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void loop() {
  _button.loop();
  _mp3Audio.update();
  M5.update();
  delay(1);
}
