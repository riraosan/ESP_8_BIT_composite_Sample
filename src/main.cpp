
#define SDFAT_FILE_TYPE 1

//#include <M5Atom.h>
#include <SPI.h>
#include <SdFat.h>
#include <Audio.hpp>
#include <Video.hpp>

Audio _mp3Audio;
Video _composit;
SdFat _SD;

bool active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  SPI.begin(23, 33, 19, -1);
  if (!_SD.begin(SdSpiConfig(-1, DEDICATED_SPI, SD_SCK_MHZ(10), &SPI))) {
    log_e("Card Mount Failed");
    return;
  } else {
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    _mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setSdFat(&_SD);
    _mp3Audio.begin();

    log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    _composit.setFilename("/non.gif");
    _composit.setSdFat(&_SD);
    _composit.begin();
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  active = true;
}

void loop() {
  if (active) {
    _mp3Audio.update();
    _composit.update();
  }
  delay(1);
}
