#ifndef ANIMATED_SPRITE_H
#define ANIMATED_SPRITE_H

#include <vector>
#include <map>
#include <string>

#include "SFML/Graphics.hpp"

class AnimatedSprite : public sf::Sprite {
private:
  std::vector<sf::Sprite*> sprites;
  std::map<std::string, sf::Sprite*> conditionals;
  sf::Clock clock;
  const float delay;
  std::string state;
  void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;
public:
  AnimatedSprite(float delay);
  void push(sf::Sprite *frame);
  void insert(const std::string &state, sf::Sprite *sprite);
  void set(const std::string &state);
  void reset();
};

#endif /* !ANIMATED_SPRITE_H */