
#include <M5Atom.h>
#include <Video.hpp>
#include <SPI.h>
#include <SD.h>

Video _composit;

bool active = false;

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 10000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _composit.setFilename("/non_small.gif");
    _composit.setSd(&SD);
    _composit.begin();
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  active = true;
}

void loop() {
  if (active) {
    _composit.update();
  }
}
