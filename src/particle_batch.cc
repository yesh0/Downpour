#include <utility>

#include "particle_batch.h"

using namespace std;
using namespace sf;

ParticleBatch::ParticleBatch(sf::Sprite sprite, float ratio)
    : sprite(std::move(sprite)), ratio(ratio) {
  setOverlap(1, 1);
}

void ParticleBatch::clear(size_t reserve) {
  vertices.clear();
  vertices.reserve(reserve * 6);
}

void ParticleBatch::add(sf::Vector2f position) {
  for (int j = 0; j != 3; ++j) {
    vertices.emplace_back(Vertex(positions[j] + position, textureCoords[j]));
  }
  for (int j = 1; j != 4; ++j) {
    vertices.emplace_back(positions[j] + position, textureCoords[j]);
  }
}

void ParticleBatches::update() {
  int i = system->GetParticleCount();
  for (auto &batch : batches) {
    batch.clear(i);
  }

  auto particles = system->GetPositionBuffer();
  auto groups = system->GetGroupBuffer();
  for (; i > 0; --i, ++particles, ++groups) {
    Vector2f position(particles->x, particles->y);
    position *= ratio;
    if (*groups == nullptr) {
      batches[0].add(position);
    } else {
      batches[1].add(position);
    }
  }
}

void ParticleBatch::draw(sf::RenderTarget &target,
                         const sf::RenderStates &states) const {
  RenderStates mine(states);
  mine.texture = sprite.getTexture();
  mine.transform *= getTransform();
  target.draw(vertices.data(), vertices.size(), sf::Triangles, mine);
}

void ParticleBatch::setOverlap(float o, float radius) {
  overlap = o;

  auto rect = sprite.getTextureRect();
  auto scale = Vector2f(radius / rect.width, radius / rect.height) * ratio;
  textureCoords[0] = Vector2f(rect.left, rect.top);
  textureCoords[1] = Vector2f(rect.left, rect.top + rect.height);
  textureCoords[2] = Vector2f(rect.left + rect.width, rect.top);
  textureCoords[3] = Vector2f(rect.left + rect.width, rect.top + rect.height);
  float width = scale.x * rect.width * overlap, height = scale.y * rect.height * overlap;
  positions[0] = Vector2f(width * -0.5, height * -0.5);
  positions[1] = Vector2f(width * -0.5, height * 0.5);
  positions[2] = Vector2f(width * 0.5, height * -0.5);
  positions[3] = Vector2f(width * 0.5, height * 0.5);
}

ParticleBatches::ParticleBatches(b2ParticleSystem *system, sf::Sprite water,
                                 sf::Sprite elastic, float ratio)
  : system(system), ratio(ratio) {
  batches.emplace_back(water, ratio);
  batches.emplace_back(elastic, ratio);
}

void ParticleBatches::draw(sf::RenderTarget &target, const sf::RenderStates &states) const {
  for (auto &batch : batches) {
    target.draw(batch, states);
  }
}

void ParticleBatches::setOverlap(float o) {
  for (auto &batch : batches) {
    batch.setOverlap(o, system->GetRadius());
  }
}