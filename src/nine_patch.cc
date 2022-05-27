#include <vector>

#include "nine_patch.h"

using namespace std;
using namespace sf;

void NinePatchSprite::draw(sf::RenderTarget &target,
                           const sf::RenderStates &states) const {
  RenderStates mine = states;
  mine.texture = texture;
  mine.transform *= getTransform();
  target.draw(vertices.data(), 22, TriangleStrip, mine);
}

NinePatchSprite::NinePatchSprite(const sf::Texture &texture,
                                 const sf::IntRect &clip,
                                 const sf::IntRect &patchCenter)
    : texture(&texture), textureRect(clip), center(patchCenter) {
  updateVertices();
}

/**
 * @brief Sets up TriangleStrip coordinates
 * 
 * @param vertices storage
 * @param xs 
 * @param ys 
 * @param xpositions 
 * @param ypositions 
 * @param count 
 * @param inc 
 * @param trail TriangleStrips are strips, set this to true when you want to set an end to it
 * @return int the vertice count that are used
 */
inline int updateHoritontalTriangleStrip(Vertex *vertices, const float *xs,
                                         const float *ys, const float *xpositions,
                                         const float *ypositions, size_t count, int inc,
                                         bool trail) {
  const float *lastx = xs + inc * (count <= 0 ? 0 : count - 1);
  const float *lastxposition = xpositions + inc * (count <= 0 ? 0 : count - 1);
  int i;
  for (i = 0; i != count; ++i, xs += inc, xpositions += inc, vertices += 2) {
    vertices[0].position = {*xpositions, ypositions[0]};
    vertices[0].texCoords = {*xs, ys[0]};
    vertices[1].position = {*xpositions, ypositions[1]};
    vertices[1].texCoords = {*xs, ys[1]};
  }
  if (trail) {
    vertices[0].position = {*lastxposition, ypositions[2]};
    vertices[0].texCoords = {*lastx, ys[2]};
    return 2 * i + 1;
  } else {
    return 2 * i;
  }
}

void NinePatchSprite::updateVertices() {
  /* I do not plan to explain OpenGL here, which I do not deem possible.
   * You should understand that OpenGL coding IS a mess.
   */

  // Three columns in a nine-patch, so four X coords for the borders for each column
  const float xs[] = {
      static_cast<float>(textureRect.left),
      static_cast<float>(textureRect.left + center.left),
      static_cast<float>(textureRect.left + center.left + center.width),
      static_cast<float>(textureRect.left + textureRect.width),
  };
  // Three rows in a nine-patch, so four Y coords for the borders for each row
  const float ys[] = {
      static_cast<float>(textureRect.top),
      static_cast<float>(textureRect.top + center.top),
      static_cast<float>(textureRect.top + center.top + center.height),
      static_cast<float>(textureRect.top + textureRect.height),
  };
  // xs, ys are in texture coordinates

  const float xscale = (size.x - textureRect.width + center.width) / (float)center.width;
  const float yscale = (size.y - textureRect.height + center.height) / (float)center.width;
  const float xscaled[] = {
      static_cast<float>(0),
      static_cast<float>(center.left),
      center.left + center.width * xscale,
      textureRect.width + center.width * (xscale - 1),
  };
  const float yscaled[] = {
      static_cast<float>(0),
      static_cast<float>(center.top),
      center.top + center.height * yscale,
      textureRect.height + center.width * (yscale - 1),
  };
  // xscaled, yscaled are in rendered coordinates

  // In OpenGL, we need both texture and rendered coordinates.
  // Not going to explain more.

  // OpenGL renders triangles as a basic unit.
  // 22 vertices in total (I counted it) is what we need to render a nine-patch in TriangleStrip.
  this->vertices.resize(22);

  Vertex *vertices = this->vertices.data();

  // With TriangleStrip, you fill the data in a SNAKE-shape
  // Sets up vertices for the three patchs in the first row
  // Fills Patch (0, 0), (0, 1), (0, 2) and the first part of (1, 2)
  vertices += updateHoritontalTriangleStrip(vertices, xs, ys, xscaled, yscaled, 4, 1, true);
  // Sets up vertices for the three patchs in the second row
  // Fills Patch the latter part of (1, 2) and  (1, 1), (1, 0) and the first part of (2, 0)
  vertices +=
      updateHoritontalTriangleStrip(vertices, xs + 2, ys + 1, xscaled + 2, yscaled + 1, 3, -1, true);
  // Sets up vertices for the three patchs in the third row
  // Fills Patch the latter part of (2, 0) and  (2, 1), (2, 2), end.
  vertices +=
      updateHoritontalTriangleStrip(vertices, xs + 1, ys + 2, xscaled + 1, yscaled + 2, 3, 1, false);
}

void NinePatchSprite::setSize(const sf::Vector2f &size) {
  this->size = size;
  updateVertices();
}

sf::Vector2f NinePatchSprite::getSize() {
  return size;
}

sf::FloatRect NinePatchSprite::getLocalBounds() const {
  FloatRect rect{Vector2f(), Vector2f(size)};
  return rect;
}