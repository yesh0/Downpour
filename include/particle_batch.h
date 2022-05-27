#ifndef PARTICLE_BATCH_H
#define PARTICLE_BATCH_H

#include <vector>

#include "SFML/Graphics.hpp"
#include "Box2D/Box2D.h"

/**
 * @brief Batch-renders the particles to speed up rendering
 * 
 */
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

/**
 * @brief Renders particles by their groups
 *
 * Currently simply renders water particles and elastic particles differently.
 */
class ParticleBatches : public sf::Drawable, public sf::Transformable {
private:
  std::vector<ParticleBatch> batches;
  b2ParticleSystem *system;
  float ratio;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;
public:
  ParticleBatches(b2ParticleSystem *system, sf::Sprite water, sf::Sprite elastic, float ratio);
  void update();
  /**
   * @brief Set the scale of the particles (i.e. how particles seem to overlap each other)
   *
   * It also initializes the inner OpenGL data
   *
   * @param o 
   */
  void setOverlap(float o);
};

#endif /* !PARTICLE_BATCH_H */