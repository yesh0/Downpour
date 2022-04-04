#include <cmath>
#include <exception>
#include <vector>

#include "SFML/System.hpp"

#include "b2_tiled.h"
#include "tiled_world.h"
#include "forward_defs.h"

using namespace sf;
using namespace std;

static Sprite &centeredSprite(Sprite &&sprite) {
  auto size = sprite.getTextureRect().getSize();
  sprite.setOrigin(Vector2f(size.x * 0.5, size.y * 0.5));
  return sprite;
}

sf::Sprite *TiledWorld::insertByName(const B2WorldInfo::TextureInfo &info,
                                     const std::string &name,
                                     bool ninePatched) {
  float scale = renDef.drawPPM / renDef.texturePPM;
  return insertByName(info, name, scale, ninePatched);
}

sf::Sprite *TiledWorld::insertByName(const B2WorldInfo::TextureInfo &info,
                                     const std::string &name, float scale,
                                     bool ninePatched) {
  Sprite *sp;
  if (ninePatched) {
    auto sprite = textureBundle.getNinePatch(name);
    float w = info.w, h = info.h;
    sprite.setSize({w, h});
    sprite.setOrigin({w * 0.5f, h * 0.5f});
    sprite.setScale(Vector2f(scale, scale));
    auto i = b2NinePatches.insert_after(b2NinePatches.before_begin(), sprite);
    sp = &*i;
  } else {
    auto sprite = textureBundle.getSprite(name);
    auto &rect = sprite.getTextureRect();
    sprite.setOrigin(Vector2f(rect.width * 0.5, rect.height * 0.5));
    sprite.setScale(
        Vector2f(info.w * scale / rect.width, info.h * scale / rect.height));
    auto i = b2Sprites.insert_after(b2Sprites.before_begin(), sprite);
    sp = &*i;
  }
  return sp;
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
  auto shaderFile = manager.getData(renDef.shader);
  sf::MemoryInputStream shaderStream;
  shaderStream.open(shaderFile->data, shaderFile->size);
  if (!shaderPass.create(renDef.screenW, renDef.screenH) ||
      !backgroundPass.create(renDef.screenW, renDef.screenH) ||
      !rainShader.loadFromStream(shaderStream, Shader::Type::Fragment)) {
    throw runtime_error("Give up");
  }
  auto &todoInfo = b2Loader.getInfo();
  for (auto &p : todoInfo.texturedObjects) {
    auto &anim = b2AnimatedSprites.emplace_back(p.second.delay);
    for (auto &name : p.second.names) {
      anim.push(insertByName(p.second, name, scale, p.second.ninePatched));
    }
    for (auto &cond : p.second.conditionals) {
      anim.insert(cond.first, insertByName(p.second, cond.second, scale,
                                           p.second.ninePatched));
    }
    anim.setOrigin(-p.second.offset * scale);
    auto body = p.first;
    auto info = B2Loader::getInfo(body);
    if (info != nullptr) {
      info->world = this;
      info->spriteId = b2AnimatedSprites.size() - 1;
    }
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
    b2AnimatedSprites[i].setPosition(Vector2f(position.x, position.y));
    b2AnimatedSprites[i].setRotation(sf::radians(body->GetAngle()));
  }
  RenderStates mine;
  mine.transform = getTransform();
  shaderPass.clear(Color::Transparent);
  shaderPass.draw(particleBatch, mine);
  shaderPass.display();
  backgroundPass.clear(Color::Transparent);
  backgroundPass.draw(map, mine);
  for (auto &sprite : b2AnimatedSprites) {
    backgroundPass.draw(sprite, mine);
  }
  backgroundPass.display();
  rainShader.setUniform("background", backgroundPass.getTexture());
  rainShader.setUniform("screenColorBuffer", rainShader.CurrentTexture);
  rainShader.setUniform("t", clock.getElapsedTime().asSeconds());
}

void TiledWorld::draw(sf::RenderTarget &target,
                      const sf::RenderStates &states) const {
  Sprite shaderPassSprite(shaderPass.getTexture());
  target.draw(shaderPassSprite, &rainShader);
}

void TiledWorld::query(b2Vec2 screenCoord, QueryCallback &callback) {
  b2AABB aabb;
  b2Vec2 worldCoord = screenCoord / renDef.drawPPM;
  b2Vec2 d = b2Vec2{1, 1} / renDef.drawPPM;
  aabb.lowerBound = worldCoord - d;
  aabb.upperBound = worldCoord + d;
  callback.setPPM(renDef.drawPPM);
  world.QueryAABB(&callback, aabb);
}

QueryCallback::QueryCallback(b2Vec2 p) : p(p) {}

void QueryCallback::setPPM(float ppm) { this->ppm = ppm; }

bool QueryCallback::ReportFixture(b2Fixture *fixture) {
  auto data = B2Loader::getInfo(fixture->GetBody());
  if (data != nullptr) {
    if (fixture->TestPoint(p / ppm)) {
      return callback(*(B2ObjectInfo *)data);
    }
  }
  return true;
}

bool QueryCallback::ShouldQueryParticleSystem(
    const b2ParticleSystem *particleSystem) {
  return false;
}

TiledWorldDef::RainDef &TiledWorld::getRainDef() { return rainDef; }
TiledWorldDef::RenDef &TiledWorld::getRenDef() { return renDef; }
b2World &TiledWorld::getWorld() { return world; }
b2ParticleSystem &TiledWorld::getParticleSystem() { return *particleSystem; }
b2Body *TiledWorld::getPlayer() { return b2Loader.getInfo().player; }

bool TiledContactFilter::ShouldCollide(b2Fixture *fixture,
                                       b2ParticleSystem *particleSystem,
                                       int32 particleIndex) {
  return fixture->GetFilterData().groupIndex >= 0;
}

bool TiledContactFilter::ShouldCollide(b2Fixture *fixtureA,
                                       b2Fixture *fixtureB) {
  auto a = B2Loader::getInfo(fixtureA->GetBody()),
       b = B2Loader::getInfo(fixtureB->GetBody());
  if (a != nullptr && b != nullptr) {
    if ((a->collisionGroup & b->collisionGroup) == 0) {
      return true;
    } else {
      return false;
    }
  }
  return true;
}
bool TiledContactFilter::ShouldCollide(b2ParticleSystem *particleSystem,
                                       int32 particleIndexA,
                                       int32 particleIndexB) {
  return true;
}

b2Body *TiledWorld::findByName(const std::string &name) {
  return b2Loader.findByName(name);
}

AnimatedSprite *TiledWorld::findSpriteByName(const std::string &name) {
  return getSprite(B2Loader::getInfo(b2Loader.findByName(name))->spriteId);
}

AnimatedSprite *TiledWorld::bindSprite(b2Body *body) {
  b2Loader.getInfo().texturedObjects.push_back(
      make_pair(body, B2WorldInfo::TextureInfo{}));
  auto info = b2Loader.bindObjectInfo(body);
  auto &anim = b2AnimatedSprites.emplace_back(1);
  info->world = this;
  info->spriteId = b2AnimatedSprites.size() - 1;
  return &anim;
}

const std::vector<b2Body *> &TiledWorld::getNodes() {
  return b2Loader.getInfo().nodes;
}

AnimatedSprite *TiledWorld::getSprite(std::size_t id) { return &b2AnimatedSprites[id]; }