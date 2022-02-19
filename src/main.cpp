
#include <M5Atom.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.hpp>

Audio _mp3Audio;

bool active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    _mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setSDFS(&SD);
    _mp3Audio.begin();
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  active = true;
}

void loop() {
  if (active) {
    _mp3Audio.update();
  }
  delay(1);
}
