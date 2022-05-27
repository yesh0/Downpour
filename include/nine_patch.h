#ifndef NINE_PATCH_H
#define NINE_PATCH_H

#include <vector>

#include "SFML/Graphics.hpp"

/**
 * @brief Nine-Patch Sprites
 *
 * See https://developer.android.google.cn/reference/android/graphics/NinePatch
 * for an introduction of the concept.
 */
class NinePatchSprite : public sf::Sprite {
private:
  std::vector<sf::Vertex> vertices;
  sf::Vector2f size;
  const sf::Texture *texture;
  sf::IntRect textureRect;
  sf::IntRect center;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;
  void updateVertices();
public:
  /**
   * @brief Construct a new Nine Patch Sprite object
   * 
   * @param texture the larger texture (bundled texture)
   * @param clip the area of a nine-patch pattern, (x0, y0, w0, h0)
   * @param patchCenter the central tile (x, y, w, h), relative to (x0, y0)
   */
  NinePatchSprite(const sf::Texture &texture, const sf::IntRect &clip,
                  const sf::IntRect &patchCenter);
  /**
   * @brief Set the size
   * 
   * @param size the (width, height) vector, the nine-patch pattern will then be scaled accordingly
   */
  void setSize(const sf::Vector2f &size);
  /**
   * @brief Get the size
   * 
   * @return sf::Vector2f the size
   */
  sf::Vector2f getSize();
  /**
   * @return sf::FloatRect Shortcut for FloatRect{Vector2f(), Vector2f(size)}
   */
  sf::FloatRect getLocalBounds() const;
};

#endif /* !NINE_PATCH_H */