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
  /**
   * @brief Construct a new Rain Mixer object
   *
   * The rain drop sound files are {first}.ogg, {first+1}.ogg, ..., {last}.ogg.
   * 
   * @param files the asset manager
   * @param first the start index of rain drop sound file series
   * @param last the last index of rain drop sound file series
   * @param pourer the file name of the downpour sound
   */
  RainMixer(AssetManager &files, int first, int last, std::string pourer);
  void update(int hits);
  /**
   * @brief Randomly plays water drop sounds
   *
   * It randomly determines when and what to play (though hard-coded),
   * so just keep calling it in the game loop.
   */
  void drop();
  /**
   * @brief Start / Stop playing the downpour sound
   * 
   * @param start true if raining (i.e. requests to start playing the sound)
   */
  void pour(bool start);
};

#endif /* !RAIN_MIXER_H */