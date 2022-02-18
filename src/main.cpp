
#define ENABLE_AUDIO

#include <Arduino.h>

#if defined(ENABLE_AUDIO)
#include <Audio.hpp>
Audio _mp3Audio;
#else
#include <Connect.hpp>
#include <Video.hpp>
Video   _composit;
Connect _wifi;

void initWiFi() {
  _wifi.setTaskName("AutoConnect");
  _wifi.setTaskSize(4096 * 2);
  _wifi.setTaskPriority(3);
  _wifi.setCore(0);
  _wifi.setHostName("atom_video");
  _wifi.setAPName("ATOM_VIDEO-G");
  _wifi.start(nullptr);
}
#endif

bool active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

#if defined(ENABLE_AUDIO)
  _mp3Audio.setBleSpeakerName("Soundcore 3");
  _mp3Audio.setUrl("https://riraosan.github.io/mp3/non.mp3");
  _mp3Audio.begin();
#else
  initWiFi();
  delay(5000);
  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _composit.setFilename("/non.gif");
    _composit.setSd(&SD);
    _composit.begin();
  }
#endif

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
