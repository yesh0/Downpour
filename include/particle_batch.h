#ifndef PARTICLE_BATCH_H
#define PARTICLE_BATCH_H

#include <vector>

#include "SFML/Graphics.hpp"
#include "Box2D/Box2D.h"

class ParticleBatch : public sf::Drawable, public sf::Transformable {
private:
  std::vector<sf::Vertex> vertices;
  sf::Sprite sprite;
  float overlap;
  float ratio;
  sf::Vector2f textureCoords[4];
  sf::Vector2f positions[4];
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;
public:
  ParticleBatch(sf::Sprite sprite, float ratio);
  void clear(size_t reserve);
  void add(sf::Vector2f position);
  void setOverlap(float o, float radius);
};

class ParticleBatches : public sf::Drawable, public sf::Transformable {
private:
  std::vector<ParticleBatch> batches;
  b2ParticleSystem *system;
  float ratio;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;
public:
  ParticleBatches(b2ParticleSystem *system, sf::Sprite water, sf::Sprite elastic, float ratio);
  void update();
  void setOverlap(float o);
};

#endif /* !PARTICLE_BATCH_H */