
#include <M5Atom.h>
#include <Video.hpp>
#include <SPI.h>
#include <SD.h>
#include <SimpleCLI.h>

Video     _composit;
SimpleCLI _cli;

bool active = false;

void commandCallback(cmd* c) {
  Command cmd(c);
  String  cmdName(cmd.getName());

  log_i("%s", cmdName.c_str());
  if (cmdName == "restart") {
    _composit.stop();
    _composit.closeGif();
    delay(1000);
    ESP.restart();
    delay(5000);
  } else if (cmdName == "start") {
    _composit.start();
  } else if (cmdName == "stop") {
    _composit.stop();
  }
}

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  Serial.begin(115200);

  _cli.addCmd("start", commandCallback);
  _cli.addCmd("stop", commandCallback);
  _cli.addCmd("restart", commandCallback);

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
  _composit.update();

  if (Serial.available() > 0) {
    _cli.parse(Serial.readStringUntil('\n'));
  }

  delay(1);
}
