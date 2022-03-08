#include <cmath>
#include <vector>

#include "tiled_world.h"

using namespace sf;
using namespace std;

static Sprite &centeredSprite(Sprite &&sprite) {
  auto size = sprite.getTextureRect().getSize();
  sprite.setOrigin(Vector2f(size.x * 0.5, size.y * 0.5));
  return sprite;
}

TiledWorld::TiledWorld(const std::string &tiledFile,
                       const std::string &textureBundleFilename,
                       const TiledWorldDef &def, AssetManager &manager)
    : rng(randomDevice()), world(def.gravity),
      textureBundle(textureBundleFilename, manager), manager(manager),
      rainDef(def.rainDef), renDef(def.rendering),
      b2Loader(world, 1 / def.rendering.texturePPM), tiledLoader(manager),
      map(tiledLoader.load(tiledFile, b2Loader)),
      particleSystem(world.CreateParticleSystem(&def.particleSystemDef)),
      particleBatch(
          particleSystem,
          centeredSprite(textureBundle.getSprite((def.particleTexture))),
          def.rendering.drawPPM) {
  particleBatch.setOverlap(renDef.rainScale);
  float scale = renDef.drawPPM / renDef.texturePPM;
  map.setScale(Vector2f(scale, scale));
  auto &todoInfo = b2Loader.getInfo();
  for (auto &p : todoInfo.texturedObjects) {
    auto sprite = textureBundle.getSprite(p.second.name);
    auto scale = renDef.drawPPM / renDef.texturePPM;
    auto &rect = sprite.getTextureRect();
    sprite.setOrigin(Vector2f(rect.width * 0.5, rect.height * 0.5));
    sprite.setScale(Vector2f(p.second.w * scale / rect.width,
                             p.second.h * scale / rect.height));
    b2Sprites.push_back(sprite);
  }
  world.SetContactFilter(&filter);
}

void TiledWorld::step(float time) {
  if (rainDef.rain) {
    float drops = rainDef.dropPerSecond * time;
    int dropCount = (int)(drops);
    if (uniform(rng) < drops - dropCount) {
      dropCount++;
    }
    b2ParticleDef pd;
    pd.flags |= b2_fixtureContactFilterParticle;
    pd.lifetime = rainDef.rainLifetime;
    pd.velocity = rainDef.rainVelocity / renDef.texturePPM;
    pd.velocity *= (2 * uniform(rng) - 1) * rainDef.rainVelocityRandom + 1;
    float angle = rainDef.rainAngularRandom * (2 * uniform(rng) - 1);
    pd.velocity = b2Mul(b2Rot(angle), pd.velocity);
    for (int i = 0; i < dropCount; ++i) {
      pd.position.Set(rainDef.rainZone.x + uniform(rng) * rainDef.rainZone.z,
                      rainDef.rainZone.y + uniform(rng) * rainDef.rainZone.w);
      pd.position = pd.position / renDef.texturePPM;
      particleSystem->CreateParticle(pd);
    }
  }
  world.Step(time, 6, 2);
  auto &joints = b2Loader.getJoints();
  auto prev = joints.before_begin();
  for (auto i = joints.begin(); i != joints.end();) {
    auto force = i->first->GetReactionForce(1).LengthSquared();
    if (force > i->second * i->second) {
      world.DestroyJoint(i->first);
      i++;
      joints.erase_after(prev);
    } else {
      prev = i;
      i++;
    }
  }
}

void TiledWorld::prepare() {
  particleBatch.update();
  auto &textured = b2Loader.getInfo().texturedObjects;
  for (int i = 0; i != textured.size(); ++i) {
    auto body = textured[i].first;
    auto position = body->GetPosition() * renDef.drawPPM;
    b2Sprites[i].setPosition(Vector2f(position.x, position.y));
    b2Sprites[i].setRotation(sf::radians(body->GetAngle()));
  }
}

void TiledWorld::draw(sf::RenderTarget &target,
                      const sf::RenderStates &states) const {
  RenderStates mine(states);
  mine.transform *= getTransform();
  target.draw(map, mine);
  for (auto &sprite : b2Sprites) {
    target.draw(sprite, mine);
  }
  target.draw(particleBatch, mine);
}

TiledWorldDef::RainDef &TiledWorld::getRainDef() { return rainDef; }

bool TiledContactFilter::ShouldCollide(b2Fixture *fixture,
                                       b2ParticleSystem *particleSystem,
                                       int32 particleIndex) {
  return fixture->GetFilterData().groupIndex >= 0;
}

bool TiledContactFilter::ShouldCollide(b2Fixture *fixtureA,
                                       b2Fixture *fixtureB) {
  return true;
}
bool TiledContactFilter::ShouldCollide(b2ParticleSystem *particleSystem,
                                       int32 particleIndexA,
                                       int32 particleIndexB) {
  return true;
}