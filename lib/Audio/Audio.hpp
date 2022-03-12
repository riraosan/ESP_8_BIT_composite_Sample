
#pragma once

#define USE_HELIX
#define USE_A2DP
#define A2DP_BUFFER_COUNT 30  // to control exception ~128Kbps

#include <M5Atom.h>
#include <AudioTools.h>
#include <AudioCodecs/CodecMP3Helix.h>
#include <SPI.h>
#include <SD.h>

using namespace audio_tools;

class Audio {
public:
  Audio() : _source(A2DPStream::instance()),
            _decoder(&_source, &_mp3decoder),
            _copy(_decoder, _audioFile),
            _bleSpeakerName(""),
            _filename(""),
            _active(false){};

  void begin() {
    // Serial.begin(115200);
    // AudioLogger::instance().begin(Serial, AudioLogger::Info);
    if (_pSD != nullptr) {
      _audioFile = _pSD->open(_filename.c_str());

      if (_audioFile.size() > 0) {
        log_i("Open Audio File...");

        _source.setVolume(1.0);
        _source.begin(TX_MODE, _bleSpeakerName.c_str());
        log_i("A2DP is connected now...");

        _decoder.setNotifyAudioChange(_source);
        _decoder.begin();
        log_i("Begin decoder...");

      } else {
        log_e("can not open mp3 file.");
      }
    }
  };

  void setSDFS(SDFS *sd) {
    _pSD = sd;
  }

  void setBleSpeakerName(String name) {
    _bleSpeakerName = name;
  }

  void setFilename(String name) {
    _filename = name;
  }

  void disconnect(void) {
    _source.source().end(false);
  }

  void setAutoReconnect(bool isEnable) {
    if (!_active)
      _source.source().set_auto_reconnect(isEnable);
  }

  void setResetBle(bool isEnable) {
    if (!_active)
      _source.source().set_reset_ble(isEnable);
  }

  void start(void) {
    _active = true;
  }

  void stop(void) {
    _active = false;
  }

  void cue(void) {
    _decoder.flush();
    _audioFile.seek(0);
  }

  void update() {
    if (_active) {
      if (!_copy.copy()) {
        _active = false;
      }
    }

    delay(1);
  }

private:
  SDFS              *_pSD;
  File               _audioFile;
  A2DPStream         _source;
  MP3DecoderHelix    _mp3decoder;
  EncodedAudioStream _decoder;
  StreamCopy         _copy;

  String _bleSpeakerName;
  String _filename;
  bool   _active;
};
