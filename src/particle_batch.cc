#include "particle_batch.h"

using namespace std;
using namespace sf;

ParticleBatch::ParticleBatch(b2ParticleSystem *system, sf::Sprite sprite, float ratio)
    : sprite(sprite), system(system), overlap(1), ratio(ratio) {}

void ParticleBatch::update() {
  auto particles = system->GetPositionBuffer();
  int i = system->GetParticleCount();
  vertices.clear();
  vertices.reserve(i * 6);
  auto rect = sprite.getTextureRect();
  auto radius = system->GetRadius();
  auto scale = Vector2f(radius / rect.width, radius / rect.height) * ratio;
  Vector2f textureCoords[] = {
      Vector2f(rect.left, rect.top),
      Vector2f(rect.left, rect.top + rect.height),
      Vector2f(rect.left + rect.width, rect.top),
      Vector2f(rect.left + rect.width, rect.top + rect.height)};
  float width = scale.x * rect.width * overlap, height = scale.y * rect.height * overlap;
  Vector2f positions[] = {Vector2f(width * -0.5, height * -0.5),
                          Vector2f(width * -0.5, height * 0.5),
                          Vector2f(width * 0.5, height * -0.5),
                          Vector2f(width * 0.5, height * 0.5)};
  for (; i > 0; --i, ++particles) {
    Vector2f position(particles->x, particles->y);
    position *= ratio;
    for (int j = 0; j != 3; ++j) {
      vertices.push_back(Vertex(positions[j] + position, textureCoords[j]));
    }
    for (int j = 1; j != 4; ++j) {
      vertices.push_back(Vertex(positions[j] + position, textureCoords[j]));
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

void ParticleBatch::setOverlap(float o) {
  overlap = o;
}