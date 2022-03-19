
#include <M5Atom.h>
#include <Video.hpp>
#include <SPI.h>
#include <SD.h>
#include <Button2.h>
#include <Ticker.h>

Video   _composit;
Button2 _button;
Ticker  _audioStart;

bool isStart = false;

void startAudio(void) {
  Serial1.print("start\n");  // to audio
}

void clickHandler(Button2& btn) {
  switch (btn.getClickType()) {
    case SINGLE_CLICK:
      log_i("single ");
      if (isStart) {
        _composit.stop();
        Serial1.print("stop\n");  // to audio
        isStart = true;
      } else {
        _composit.start();
        _audioStart.once_ms(1150, startAudio);
        isStart = false;
      }
      break;
    case DOUBLE_CLICK:
      log_i("double ");
      break;
    case TRIPLE_CLICK:
      log_i("triple ");
      break;
    case LONG_CLICK:
      log_i("long");
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

  Serial1.begin(115200, SERIAL_8N1, 26, 32);
  Serial1.flush();

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
  if(!_composit.state()){
    _button.loop();
  }
  _composit.update();
}
