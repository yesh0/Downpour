#ifndef PARTICLE_BATCH_H
#define PARTICLE_BATCH_H

#include <vector>

#include "SFML/Graphics.hpp"
#include "Box2D/Box2D.h"

class ParticleBatch : public sf::Drawable, public sf::Transformable {
private:
  std::vector<sf::Vertex> vertices;
  b2ParticleSystem *system;
  sf::Sprite sprite;
  float overlap;
  float ratio;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;
public:
  ParticleBatch(b2ParticleSystem *system, sf::Sprite sprite, float ratio);
  void update();
  void setOverlap(float o);
};

#endif /* !PARTICLE_BATCH_H */