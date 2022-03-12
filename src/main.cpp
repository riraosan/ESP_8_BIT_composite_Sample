
#include <M5Atom.h>
#include <Video.hpp>
#include <SPI.h>
#include <SD.h>
#include <Button2.h>

Video     _composit;
SimpleCLI _cli;
Button2   _button;

bool active = false;

void clickHandler(Button2& btn) {
  switch (btn.getClickType()) {
    case SINGLE_CLICK:
      log_i("single ");

      Serial.print("start\n");  // to video
      break;
    case DOUBLE_CLICK:
      log_i("double ");
      break;
    case TRIPLE_CLICK:
      log_i("triple ");

      Serial.print("cue\n");  // to video
      break;
    case LONG_CLICK:
      log_i("long");

      Serial.print("restart\n");  // to video
      break;
  }
  log_i("click (%d)", btn.getNumberOfClicks());
}

void initButton(void) {
  _button.setClickHandler(clickHandler);
  _button.setDoubleClickHandler(clickHandler);
  _button.setTripleClickHandler(clickHandler);
  _button.setLongClickHandler(clickHandler);
  _button.setDebounceTime(10);
  _button.begin(39);  // for ATOM Lite
}

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  initButton();

  Serial.begin(115200);

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 24000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _composit.setFilename("/non_small.gif");
    _composit.setSd(&SD);
    _composit.begin();
    _composit.openGif();
  }

  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void loop() {
  _button.loop();
  _composit.update();

  delay(1);
}
