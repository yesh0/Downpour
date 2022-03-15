#ifndef TILED_WORLD_H
#define TILED_WORLD_H

#include <string>
#include <random>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"
#include "SFML/System/Clock.hpp"
#include "asset_manager.h"
#include "tiled.h"
#include "b2_tiled.h"
#include "particle_batch.h"
#include "nine_patch.h"

struct TiledWorldDef {
  std::string particleTexture;
  struct RenDef {
    /* Pixel per meter */
    /* Positinos:
     * Tiled maps --texturePPM--> b2World objects
     * b2World objects --drawPPM--> Rendered image
     *
     * Texture size:
     * rainScale: scales textures of each rain drop
     */
    float texturePPM, drawPPM, rainScale;
    size_t screenW, screenH;
  } rendering;
  b2ParticleSystemDef particleSystemDef;
  b2Vec2 gravity;

  struct RainDef {
    /* Particles */
    /* Zone to spawn particles, (x, y, w, h) */
    b2Vec4 rainZone;
    b2Vec2 rainVelocity;
    /* Angular randomization */
    float rainAngularRandom;
    /* Initial velocity randomization */
    float rainVelocityRandom;
    float rainLifetime;
    float dropPerSecond;
    bool rain;
  } rainDef;
};

class TiledContactFilter : public b2ContactFilter {
public:
  bool ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB);
  bool ShouldCollide(b2Fixture *fixture, b2ParticleSystem *particleSystem,
                     int32 particleIndex);
  bool ShouldCollide(b2ParticleSystem *particleSystem, int32 particleIndexA,
                     int32 particleIndexB);
};

class QueryCallback : public b2QueryCallback {
protected:
  const b2Vec2 p;
  float ppm;
  QueryCallback(b2Vec2 p);
public:
  virtual bool callback(const std::string &name) = 0;
  bool ReportFixture(b2Fixture* fixture);
  bool ShouldQueryParticleSystem(const b2ParticleSystem* particleSystem);
  void setPPM(float ppm);
};

class TiledWorld : public sf::Drawable, public sf::Transformable {
private:
  sf::Clock clock;
  std::random_device randomDevice;
  std::mt19937 rng;
  std::uniform_real_distribution<> uniform;
  sf::RenderTexture shaderPass;
  sf::RenderTexture backgroundPass;
  sf::Shader rainShader;
  AssetManager &manager;
  BundledTexture textureBundle;
  TiledContactFilter filter;
  b2World world;
  B2Loader b2Loader;
  TiledLoader tiledLoader;
  TiledMap map;
  b2ParticleSystem *particleSystem;
  ParticleBatch particleBatch;
  std::vector<sf::Sprite*> b2SpritePointers;
  std::forward_list<sf::Sprite> b2Sprites;
  std::forward_list<NinePatchSprite> b2NinePatches;
  TiledWorldDef::RainDef rainDef;
  TiledWorldDef::RenDef renDef;
  void draw(sf::RenderTarget& target, const sf::RenderStates& states) const;

public:
  TiledWorld(const std::string &tiledFile,
             const std::string &textureBundleFilename, const TiledWorldDef &def,
             AssetManager &manager);
  /* Steps the b2World */
  void step(float time);
  /* Populates vertices
   * Workaround since sf::Drawable::draw is const.
   * To be called before rendering
   */
  void prepare();
  TiledWorldDef::RainDef &getRainDef();
  void query(b2Vec2 screenCoord, QueryCallback &callback);
};

#endif /* !TILED_WORLD_H */