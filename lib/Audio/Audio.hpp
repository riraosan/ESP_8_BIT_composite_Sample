
#pragma once

#define USE_HELIX
#define USE_A2DP

#include <Arduino.h>
#include <AudioTools.h>
#include <AudioCodecs/CodecMP3Helix.h>
#include <secrets.h>

using namespace audio_tools;

class Audio {
public:
  Audio() : _source(A2DPStream::instance()),
            _decoder(&_source, &_mp3decoder),
            _urlStream(SECRET_SSID, SECRET_PASS),
            _copy(_decoder, _urlStream),
            _bleSpeakerName(""),
            _filename(""),
            _url(""),
            _active(false){

            };

  void begin() {
    // Serial.begin(115200);
    // AudioLogger::instance().begin(Serial, AudioLogger::Info);

    _source.setVolume(1.0);
    _source.begin(TX_MODE, _bleSpeakerName.c_str());
    log_i("A2DP is connected now...");
    log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    _decoder.setNotifyAudioChange(_source);
    _decoder.begin();
    log_i("Begin decoder...");
    log_i("Free Heap : %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    if (_urlStream.begin(_url.c_str(), "audio/mp3")) {
      _active = true;
    }
  }

  void setBleSpeakerName(String name) {
    _bleSpeakerName = name;
  }

  void setUrl(String url) {
    _url = url;
  }

  void update() {
    if (_active) {
      if (!_copy.copy()) {
        stop();
      }
    }
    delay(1);
  }

private:
  A2DPStream         _source;
  MP3DecoderHelix    _mp3decoder;
  EncodedAudioStream _decoder;
  URLStream          _urlStream;
  StreamCopy         _copy;

  String _bleSpeakerName;
  String _filename;
  String _url;
  bool   _active;
};
