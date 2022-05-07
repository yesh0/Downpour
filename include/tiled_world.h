#ifndef TILED_WORLD_H
#define TILED_WORLD_H

#include <random>
#include <string>
#include <vector>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"
#include "SFML/System/Clock.hpp"
#include "animated_sprite.h"
#include "asset_manager.h"
#include "b2_tiled.h"
#include "nine_patch.h"
#include "particle_batch.h"
#include "tiled.h"

struct TiledWorldDef {
  std::string particleTexture;
  std::string elasticParticleTexture;
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
    std::string shader;
  } rendering;
  b2ParticleSystemDef particleSystemDef;
  b2Vec2 gravity;

  struct RainDef {
    /* Particles */
    /* Zone to spawn particles, (x, y, w, h) */
    std::vector<b2Vec4> rainZones;
    float totalZoneArea;
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
  virtual bool callback(B2ObjectInfo &info) = 0;
  bool ReportFixture(b2Fixture *fixture);
  bool ShouldQueryParticleSystem(const b2ParticleSystem *particleSystem);
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
  b2ParticleSystem *particleSystem;
  B2Loader b2Loader;
  TiledLoader tiledLoader;
  TiledMap map;
  ParticleBatches particleBatches;
  std::forward_list<sf::Sprite> b2Sprites;
  std::forward_list<NinePatchSprite> b2NinePatches;
  std::vector<AnimatedSprite> b2AnimatedSprites;
  std::map<std::string, std::size_t> namedSprites;
  TiledWorldDef::RainDef rainDef;
  TiledWorldDef::RenDef renDef;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;

public:
  TiledWorld(const std::string &tiledFile,
             const std::string &textureBundleFilename, const TiledWorldDef &def,
             AssetManager &manager);
  /* Steps the b2World */
  void step(float time);
  /**
   * @brief Populates vertices
   * Workaround since sf::Drawable::draw is const.
   * To be called before rendering
   */
  void prepare();
  TiledWorldDef::RainDef &getRainDef();
  TiledWorldDef::RenDef &getRenDef();
  b2World &getWorld();
  b2ParticleSystem &getParticleSystem();
  b2Body *getPlayer();
  void query(b2Vec2 screenCoord, QueryCallback &callback);
  b2Body *findByName(const std::string &name);
  AnimatedSprite *findSpriteByName(const std::string &name);
  sf::Sprite *insertByName(const B2WorldInfo::TextureInfo &info,
                           const std::string &name, float scale,
                           bool ninePatched);
  sf::Sprite *insertByName(const B2WorldInfo::TextureInfo &info,
                           const std::string &name, bool ninePatched);
  AnimatedSprite *bindSprite(b2Body *body);
  const std::vector<b2Body *> &getNodes();
  AnimatedSprite *getSprite(std::size_t id);
};

#endif /* !TILED_WORLD_H */