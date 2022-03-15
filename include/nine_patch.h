#ifndef NINE_PATCH_H
#define NINE_PATCH_H

#include <vector>

#include "SFML/Graphics.hpp"

class NinePatchSprite : public sf::Sprite {
private:
  std::vector<sf::Vertex> vertices;
  sf::Vector2i size;
  const sf::Texture *texture;
  const sf::IntRect textureRect;
  const sf::IntRect center;
  virtual void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;
  void updateVertices();
public:
  NinePatchSprite(const sf::Texture &texture, const sf::IntRect &clip,
                  const sf::IntRect &patchCenter);
  void setSize(const sf::Vector2i &size);
};

#endif /* !NINE_PATCH_H */