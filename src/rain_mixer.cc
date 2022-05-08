#include <string>
#include <stdexcept>

#include "rain_mixer.h"

RainMixer::RainMixer(AssetManager &files, int first, int last, std::string pourer)
  : rng(std::random_device{}()), nextDrop(0), stopped(false) {
  for (int i = first; i <= last; ++i) {
    auto &soundBuffer = soundBuffers.emplace_back();
    auto filename = std::to_string(i) + ".ogg";
    auto data = files.getData(filename);
    if (!soundBuffer.loadFromMemory(data->data, data->size)) {
      throw std::runtime_error("Unable to load sound");
    }
  }
  auto pourData = files.getData(pourer);
  if (!rainPourMusic.openFromMemory(pourData->data, pourData->size)) {
    throw std::runtime_error("Unable to open rainpour sound");
  }
}

void RainMixer::drop() {
  if (lastDrop.getElapsedTime().asSeconds() >= nextDrop) {
    sf::Sound &sound = sounds.emplace_back();
    std::uniform_int_distribution<> dist(0, soundBuffers.size() - 1);
    sound.setBuffer(soundBuffers[dist(rng)]);
    sound.play();
    if (sounds.size() > 32) {
      sounds.pop_front();
    }
    lastDrop.restart();
    nextDrop = std::uniform_real_distribution<>(0, 3)(rng);
  }
  float poured = pourUpdate.getElapsedTime().asSeconds();
  if (poured < 1) {
    if (pouring) {
      rainPourMusic.setVolume(poured * 100);
    } else {
      rainPourMusic.setVolume(100 - poured * 100);
    }
  } else {
    if (!pouring && !stopped) {
      rainPourMusic.stop();
      rainPourMusic.setPlayingOffset(sf::Time());
      stopped = true;
    }
  }
}

void RainMixer::pour(bool start) {
  if (start != pouring) {
    pourUpdate.restart();
    if (start) {
      rainPourMusic.setLoop(true);
      rainPourMusic.play();
    } else {
      stopped = false;
    }
    pouring = start;
  }
}