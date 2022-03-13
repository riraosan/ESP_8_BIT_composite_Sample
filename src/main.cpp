
#include <M5Atom.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.hpp>
#include <SimpleCLI.h>
#include <Ticker.h>

Audio     _mp3Audio;
SimpleCLI _cli;

void commandCallback(cmd* c) {
  Command cmd(c);
  String  cmdName(cmd.getName());

  log_i("%s", cmdName.c_str());
  if (cmdName == "start") {
    //delay(780);
    _mp3Audio.start();

    M5.dis.drawpix(0, 0x00FF00);
  } else if (cmdName == "stop") {
    _mp3Audio.stop();

    M5.dis.drawpix(0, 0xFF0000);
  } else if (cmdName == "cue") {
    _mp3Audio.cue();

    M5.dis.drawpix(0, 0x000000);
    delay(100);
    M5.dis.drawpix(0, 0x00FF00);
    delay(100);
    M5.dis.drawpix(0, 0x000000);
    delay(100);
    M5.dis.drawpix(0, 0x00FF00);
  } else if (cmdName == "restart") {
    _mp3Audio.stop();
    _mp3Audio.disconnect();
    delay(1500);
    M5.dis.drawpix(0, 0x000000);

    ESP.restart();
    delay(2000);
  }
}

void setup() {
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  Serial1.begin(115200);

  Serial1.begin(115200, SERIAL_8N1, 32, 26);
  Serial1.flush();

  _cli.addCmd("start", commandCallback);
  _cli.addCmd("stop", commandCallback);
  //st_cli.addCmd("cue", commandCallback);
  //_cli.addCmd("restart", commandCallback);

  M5.begin(false, false, true);  // using LED
  M5.dis.drawpix(0, 0xFF0000);

  SPI.begin(23, 33, 19, -1);
  if (!SD.begin(-1, SPI, 24000000)) {
    log_e("Card Mount Failed");
    return;
  } else {
    _mp3Audio.setFilename("/non.mp3");
    _mp3Audio.setSDFS(&SD);

    _mp3Audio.setAutoReconnect(false);
    _mp3Audio.setNVSInit(true);
    _mp3Audio.setResetBle(true);
    _mp3Audio.setBleSpeakerName("Soundcore 3");
    _mp3Audio.begin();
  }

  M5.dis.drawpix(0, 0xFFFFFF);
  log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void loop() {
  _mp3Audio.update();

  if (Serial1.available() > 0) {
    String command(Serial1.readStringUntil('\n'));
    log_i("%s", command.c_str());
    _cli.parse(command);
  }

  delay(1);
}
