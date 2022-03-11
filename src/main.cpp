
#include <M5Atom.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.hpp>

Audio _mp3Audio;

bool _active = false;
bool _once   = true;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  Serial.begin(115200);

  M5.begin(false, false, true);  // use LED
  M5.dis.drawpix(0, 0x000000);

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

  M5.dis.drawpix(0, 0xFF0000);
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void loop() {
  if (M5.Btn.pressedFor(1500)) {
    _active = false;
    M5.dis.drawpix(0, 0x000000);
    _mp3Audio.disconnect();
    delay(2000);
    ESP.restart();
  }

  if (M5.Btn.wasPressed()) {
    if (_active == false) {
      _active = true;
      M5.dis.drawpix(0, 0x00FF00);
    }
  }

  if (_active){
    if(_once){
      Serial.print("start\n");
      _once = false;
      delay(800);
      Serial.print("stop\n");
    }
    _mp3Audio.update();
  }

  M5.update();
  delay(1);
}
