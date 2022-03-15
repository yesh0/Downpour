#include <cmath>
#include <exception>
#include <vector>

#include "SFML/System.hpp"

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
  auto shaderFile = manager.getData("rain.glsl");
  sf::MemoryInputStream shaderStream;
  shaderStream.open(shaderFile->data, shaderFile->size);
  if (!shaderPass.create(renDef.screenW, renDef.screenH) ||
      !backgroundPass.create(renDef.screenW, renDef.screenH) ||
      !rainShader.loadFromStream(shaderStream, Shader::Type::Fragment)) {
    throw runtime_error("Give up");
  }
  auto &todoInfo = b2Loader.getInfo();
  for (auto &p : todoInfo.texturedObjects) {
    if (p.second.ninePatched) {
      auto sprite = textureBundle.getNinePatch(p.second.name);
      float w = p.second.w, h = p.second.h;
      sprite.setSize({(int)w, (int)h});
      sprite.setOrigin({p.second.w * 0.5f, p.second.h * 0.5f});
      sprite.setScale(Vector2f(scale, scale));
      auto i = b2NinePatches.insert_after(b2NinePatches.before_begin(), sprite);
      b2SpritePointers.push_back(&(*i));
    } else {
      auto sprite = textureBundle.getSprite(p.second.name);
      auto &rect = sprite.getTextureRect();
      sprite.setOrigin(Vector2f(rect.width * 0.5, rect.height * 0.5));
      sprite.setScale(Vector2f(p.second.w * scale / rect.width,
                               p.second.h * scale / rect.height));
      auto i = b2Sprites.insert_after(b2Sprites.before_begin(), sprite);
      b2SpritePointers.push_back(&(*i));
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
    b2SpritePointers[i]->setPosition(Vector2f(position.x, position.y));
    b2SpritePointers[i]->setRotation(sf::radians(body->GetAngle()));
  }
  RenderStates mine;
  mine.transform = getTransform();
  shaderPass.clear(Color::Transparent);
  shaderPass.draw(particleBatch, mine);
  shaderPass.display();
  backgroundPass.clear(Color::Transparent);
  backgroundPass.draw(map, mine);
  for (auto sprite : b2SpritePointers) {
    backgroundPass.draw(*sprite, mine);
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

void QueryCallback::setPPM(float ppm) {
  this->ppm = ppm;
}

bool QueryCallback::ReportFixture(b2Fixture *fixture) {
  auto data = fixture->GetBody()->GetUserData();
  if (data != nullptr) {
    if (fixture->TestPoint(p / ppm)) {
      return callback(*(const string*) data);
    }
  }
  return true;
}

bool QueryCallback::ShouldQueryParticleSystem(
    const b2ParticleSystem *particleSystem) {
  return false;
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