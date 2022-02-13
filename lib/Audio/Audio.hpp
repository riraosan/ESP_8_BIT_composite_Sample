
#pragma once

#define USE_HELIX
#define USE_A2DP

#include <M5Atom.h>
#include <AudioTools.h>
#include <AudioCodecs/CodecMP3Helix.h>
#include <SPI.h>
#include <SdFat.h>

using namespace audio_tools;

class Audio {
public:
  Audio() : _source(A2DPStream::instance()),
            _decoder(&_source, &_mp3decoder),
            _copy(_decoder, _audioFile),
            _bleSpeakerName(""),
            _filename(""),
            _active(false){

            };

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
        if (_source.isConnected()) {
          _decoder.begin();
          log_i("Begin decoder...");
          _active = true;
        } else {
          log_e("isConnected() is false");
        }

      } else {
        _active = false;
        log_e("can not open mp3 file.");
      }
    }
  };

  void setSdFat(SdFat *sd) {
    _pSD = sd;
  }

  void setBleSpeakerName(String name) {
    _bleSpeakerName = name;
  }

  void setFilename(String name) {
    _filename = name;
  }

  void update() {
    if (_active) {
      if (!_copy.copy()) {
        stop();
      }
    }
  }

private:
  SdFat             *_pSD;
  File               _audioFile;
  A2DPStream         _source;
  MP3DecoderHelix    _mp3decoder;
  EncodedAudioStream _decoder;
  StreamCopy         _copy;

  String _bleSpeakerName;
  String _filename;
  bool   _active;
};
