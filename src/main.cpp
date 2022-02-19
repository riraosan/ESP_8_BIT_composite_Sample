
#include <M5Atom.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.hpp>

Audio _mp3Audio;

bool _active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  M5.begin(false, false, true);  // use LED
  M5.dis.drawpix(0, 0xFF0000);

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    //_mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setFilename("/asa-ga-kuru.mp3");
    _mp3Audio.setSDFS(&SD);
    _mp3Audio.begin();
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  M5.dis.drawpix(0, 0x00FF00);
}

void loop() {
  if (M5.Btn.pressedFor(1000)) {
    M5.dis.drawpix(0, 0x000000);
    _mp3Audio.disconnect();
    ESP.restart();
    delay(1000);
  }

  if (M5.Btn.wasPressed()) {
    if (_active) {
      _active = false;
      M5.dis.drawpix(0, 0x0000FF);
    } else {
      _active = true;
      M5.dis.drawpix(0, 0x00FF00);
    }
  }

  if (_active) {
    _mp3Audio.update();
  }

  M5.update();
  delay(1);
}
