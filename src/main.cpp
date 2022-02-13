
#define SDFAT_FILE_TYPE 1
#define ENABLE_AUDIO

#include <M5Atom.h>
#include <SPI.h>
#include <SdFat.h>

#if defined(ENABLE_AUDIO)
#include <Audio.hpp>
Audio _mp3Audio;
#else
#include <Video.hpp>
Video _composit;
#endif

SdFat _SD;

bool active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  SPI.begin(23, 33, 19, -1);
  if (!_SD.begin(SdSpiConfig(-1, DEDICATED_SPI, SD_SCK_MHZ(10), &SPI))) {
    log_e("Card Mount Failed");
    return;
  } else {
#if defined(ENABLE_AUDIO)
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    _mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setSdFat(&_SD);
    _mp3Audio.begin();
#else
    _composit.setFilename("/non.gif");
    _composit.setSdFat(&_SD);
    _composit.begin();
#endif
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  active = true;
}

void loop() {
  if (active) {
#if defined(ENABLE_AUDIO)
    _mp3Audio.update();
#else
    _composit.update();
#endif
  }
}
