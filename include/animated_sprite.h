#ifndef ANIMATED_SPRITE_H
#define ANIMATED_SPRITE_H

#include <vector>
#include <map>
#include <string>

#include "SFML/Graphics.hpp"

/**
 * @brief A collection of Sprites that plays sequentially (or nominated by state)
 *
 * When the state is "" (empty), it plays the animation.
 * When a state is set, the sprite is fixed to the corresponding one.
 */
class AnimatedSprite : public sf::Sprite {
private:
  std::vector<sf::Sprite*> sprites;
  std::map<std::string, sf::Sprite*> conditionals;
  sf::Clock clock;
  const float delay;
  std::string state;
  void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;
public:
  /**
   * @brief Construct a new Animated Sprite object
   * 
   * @param delay the animation delay, in seconds
   */
  AnimatedSprite(float delay);
  /**
   * @brief Adds a animation frame
   * 
   * @param frame the sprite
   */
  void push(sf::Sprite *frame);
  /**
   * @brief Adds a nominated frame
   * 
   * @param state the state
   * @param sprite the corresponding sprite
   */
  void insert(const std::string &state, sf::Sprite *sprite);
  /**
   * @brief Sets / Clears the state
   * 
   * @param state the state, pass an empty one ("") to clear the state
   */
  void set(const std::string &state);
  /**
   * @brief Shortcut to set("");
   */
  void reset();
};

#endif /* !ANIMATED_SPRITE_H */