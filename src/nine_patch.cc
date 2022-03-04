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
  const float xs[] = {
      static_cast<float>(textureRect.left),
      static_cast<float>(textureRect.left + center.left),
      static_cast<float>(textureRect.left + center.left + center.width),
      static_cast<float>(textureRect.left + textureRect.width),
  };
  const float ys[] = {
      static_cast<float>(textureRect.top),
      static_cast<float>(textureRect.top + center.top),
      static_cast<float>(textureRect.top + center.top + center.height),
      static_cast<float>(textureRect.top + textureRect.height),
  };
  const float xscale = (size.x - textureRect.width + center.width) / (float)center.width;
  const float yscale = (size.y - textureRect.height + center.height) / (float)center.width;
  const float xscaled[] = {
      static_cast<float>(textureRect.left),
      static_cast<float>(textureRect.left + center.left),
      textureRect.left + center.left + center.width * xscale,
      textureRect.left + textureRect.width + center.width * (xscale - 1),
  };
  const float yscaled[] = {
      static_cast<float>(textureRect.top),
      static_cast<float>(textureRect.top + center.top),
      textureRect.top + center.top + center.height * yscale,
      textureRect.top + textureRect.height + center.width * (yscale - 1),
  };
  this->vertices.resize(22);
  Vertex *vertices = this->vertices.data();
  vertices += updateHoritontalTriangleStrip(vertices, xs, ys, xscaled, yscaled, 4, 1, true);
  vertices +=
      updateHoritontalTriangleStrip(vertices, xs + 2, ys + 1, xscaled + 2, yscaled + 1, 3, -1, true);
  vertices +=
      updateHoritontalTriangleStrip(vertices, xs + 1, ys + 2, xscaled + 1, yscaled + 2, 3, 1, false);
}

void NinePatchSprite::setSize(const sf::Vector2i &size) {
  this->size = size;
  updateVertices();
}