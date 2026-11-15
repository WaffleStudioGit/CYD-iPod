#pragma once

#include <Arduino.h>
#include <vector>
#include <SD.h>

class Playlist {
 public:
  bool begin(const char* rootPath);
  bool empty() const { return tracks_.empty(); }
  size_t size() const { return tracks_.size(); }
  int index() const { return index_; }
  const String& current() const;
  const char* currentName() const;

  void next();
  void prev();
  void restartCurrent() { /* path unchanged; player seeks 0 */ }
  bool setIndex(int i);

 private:
  std::vector<String> tracks_;
  int index_ = 0;
  String empty_;

  void scanDir(File dir, const String& prefix);
  static bool isAudio(const String& name);
};
