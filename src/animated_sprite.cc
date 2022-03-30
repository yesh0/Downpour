#include <cmath>
#include <cstddef>

#include "animated_sprite.h"

void AnimatedSprite::draw(sf::RenderTarget &target,
                          const sf::RenderStates &states) const {
  sf::RenderStates mine{states};
  mine.transform *= getTransform();
  if (!state.empty()) {
    auto i = conditionals.find(state);
    if (i != conditionals.end()) {
      target.draw(*(i->second), mine);
      return;
    }
  }
  std::size_t sprite =
      (std::size_t)(clock.getElapsedTime().asSeconds() / delay) %
      sprites.size();
  target.draw(*sprites[sprite], mine);
}

AnimatedSprite::AnimatedSprite(float delay) : delay(delay), sprites(0) {}

void AnimatedSprite::push(sf::Sprite *frame) { sprites.push_back(frame); }

void AnimatedSprite::insert(const std::string &state, sf::Sprite *frame) {
  conditionals.insert(std::make_pair(state, frame));
}

void AnimatedSprite::set(const std::string &state) { this->state = state; }

void AnimatedSprite::reset() {
  this->state.clear();
  clock.restart();
}
