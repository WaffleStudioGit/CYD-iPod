#include "playlist.h"
#include "config.h"

bool Playlist::isAudio(const String& name) {
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".mp3") || lower.endsWith(".wav");
}

void Playlist::scanDir(File dir, const String& prefix) {
  if (!dir) return;
  for (;;) {
    File entry = dir.openNextFile();
    if (!entry) break;
    String name = entry.name();
    // Skip AppleDouble / hidden
    if (name.startsWith(".")) {
      entry.close();
      continue;
    }
    if (entry.isDirectory()) {
      String sub = prefix + "/" + name;
      scanDir(entry, sub);
    } else if (isAudio(name)) {
      tracks_.push_back(prefix + "/" + name);
    }
    entry.close();
  }
}

bool Playlist::begin(const char* rootPath) {
  tracks_.clear();
  index_ = 0;
  File root = SD.open(rootPath);
  if (!root || !root.isDirectory()) {
    // Fallback: scan SD root for audio files
    root = SD.open("/");
    if (!root) return false;
    scanDir(root, "");
    root.close();
    return !tracks_.empty();
  }
  scanDir(root, String(rootPath));
  root.close();
  return !tracks_.empty();
}

const String& Playlist::current() const {
  if (tracks_.empty()) return empty_;
  return tracks_[(size_t)index_];
}

const char* Playlist::currentName() const {
  if (tracks_.empty()) return "(no tracks)";
  const String& p = current();
  int slash = p.lastIndexOf('/');
  if (slash >= 0 && slash + 1 < (int)p.length()) {
    return p.c_str() + slash + 1;
  }
  return p.c_str();
}

void Playlist::next() {
  if (tracks_.empty()) return;
  index_ = (index_ + 1) % (int)tracks_.size();
}

void Playlist::prev() {
  if (tracks_.empty()) return;
  index_ = (index_ - 1 + (int)tracks_.size()) % (int)tracks_.size();
}

bool Playlist::setIndex(int i) {
  if (i < 0 || i >= (int)tracks_.size()) return false;
  index_ = i;
  return true;
}
