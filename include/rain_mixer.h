#ifndef RAIN_MIXER_H
#define RAIN_MIXER_H

#include <vector>
#include <string>
#include <list>
#include <random>

#include "SFML/Audio.hpp"

#include "asset_manager.h"

class RainMixer {
  std::vector<sf::SoundBuffer> soundBuffers;
  std::mt19937 rng;
  std::list<sf::Sound> sounds;
  sf::Clock lastDrop;
  sf::Music rainPourMusic;
  float nextDrop;
  bool pouring;
  sf::Clock pourUpdate;
  bool stopped;
public:
  RainMixer(AssetManager &files, int first, int last, std::string pourer);
  void update(int hits);
  void drop();
  void pour(bool start);
};

#endif /* !RAIN_MIXER_H */