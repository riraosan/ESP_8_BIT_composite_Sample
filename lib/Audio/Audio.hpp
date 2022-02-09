
#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>
#include <M5Atom.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <AudioTools.h>

using namespace audio_tools;
#include <AudioCodecs/CodecMP3Helix.h>

class Audio : public Task {
 public:
  Audio() : _source(A2DPStream::instance()),
            _decoder(&_source, new MP3DecoderHelix()),
            _bleSpeakerName("TaoTronics TT-BR06"),
            _filename("/non.mp3"){};

  void begin(void) {
    // Serial.begin(115200);
    // AudioLogger::instance().begin(Serial, AudioLogger::Info);

    if (SD.cardType() == CARD_SDHC) {
      _source.setVolume(8);
      _source.begin(TX_MODE, _bleSpeakerName.c_str());
      log_i("A2DP is connected now...");

      _audioFile = SD.open(_filename.c_str());
      log_i("Open Audio File...");

      _decoder.setNotifyAudioChange(_source);
      _decoder.begin();
      log_i("Begin decoder...");

      _copy.begin(_decoder, _audioFile);
      log_i("Begin copy _audioFile to decoder...");

      auto info = _decoder.decoder().audioInfo();
      log_i("The audio rate     from the mp3 file is %d", info.sample_rate);
      log_i("The channels       from the mp3 file is %d", info.channels);
      log_i("The bit per sample from the mp3 file is %d", info.bits_per_sample);
    } else {
      log_e("Please mount SDHC");
    }
  };

  void run(void* data) {
    while (1) {
      // long start = millis();
      if (!_copy.copy()) {
        log_i("Stop playing");
      }
      // log_i("copy time = %d[ms]", millis() - start);
      delay(1);
    }
  }

 private:
  A2DPStream _source;
  EncodedAudioStream _decoder;
  StreamCopy _copy;
  File _audioFile;

  String _bleSpeakerName;
  String _filename;
};
