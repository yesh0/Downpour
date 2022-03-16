#include "SFML/Graphics.hpp"
#include "pugixml.hpp"

#include "level.h"

using namespace sf;
using namespace std;
using namespace pugi;

static void loadParticleSystemDef(b2ParticleSystemDef &def, xml_node node) {
  def.colorMixingStrength = node.attribute("color-mixing").as_float();
  def.dampingStrength = node.attribute("damping").as_float();
  def.density = node.attribute("density").as_float(1);
  def.destroyByAge = node.attribute("destroy-by-age").as_bool(true);
  def.ejectionStrength = node.attribute("ejection").as_float();
  def.elasticStrength = node.attribute("elastic").as_float();
  def.gravityScale = node.attribute("gravity-scale").as_float();
  def.maxCount = node.attribute("max-count").as_int(1024);
  def.powderStrength = node.attribute("powder").as_float();
  def.pressureStrength = node.attribute("pressure").as_float();
  def.radius = node.attribute("radius").as_float();
  def.repulsiveStrength = node.attribute("repulsive").as_float();
  def.springStrength = node.attribute("spring").as_float();
  def.staticPressureIterations = node.attribute("static-iterations").as_int();
  def.staticPressureRelaxation = node.attribute("static-relaxation").as_float();
  def.staticPressureStrength = node.attribute("static").as_float();
  def.strictContactCheck = node.attribute("strict-contact").as_bool();
  def.surfaceTensionPressureStrength =
      node.attribute("surface-tension").as_float();
  def.surfaceTensionNormalStrength =
      node.attribute("surface-normal").as_float();
  def.viscousStrength = node.attribute("viscous").as_float();
}

static void loadRenDef(TiledWorldDef::RenDef &def, xml_node node) {
  auto ppm = node.child("scale");
  def.texturePPM = ppm.attribute("texture").as_float();
  def.rainScale = ppm.attribute("rain").as_float();
}

static void loadRainDef(TiledWorldDef::RainDef &def, xml_node node) {
  def.rain = node.attribute("rain").as_bool();
  def.rainLifetime = node.attribute("lifetime").as_float();
  def.dropPerSecond = node.attribute("drop-per-second").as_float();
  def.rainAngularRandom = node.attribute("angular-random").as_float();
  def.rainVelocityRandom = node.attribute("velocity-random").as_float();
  {
    auto v = node.child("velocity");
    def.rainVelocity = {v.attribute("x").as_float(),
                        v.attribute("y").as_float()};
  }
  {
    auto zone = node.child("zone");
    def.rainZone = {
        zone.attribute("x").as_float(), zone.attribute("y").as_float(),
        zone.attribute("w").as_float(), zone.attribute("h").as_float()};
  }
}

static void loadWorldDef(TiledWorldDef &def, xml_node node,
                         const TiledWorldDef::RenDef &rendering) {
  def.particleTexture = node.attribute("particle-texture").value();
  {
    auto gravity = node.child("gravity");
    def.gravity = {gravity.attribute("x").as_float(),
                   gravity.attribute("y").as_float()};
  }
  loadParticleSystemDef(def.particleSystemDef, node.child("particles"));
  def.rendering = rendering;
  loadRenDef(def.rendering, node.child("rendering"));
  loadRainDef(def.rainDef, node.child("rain"));
}

static TiledWorld *newTiledWorld(xml_node node, AssetManager &assets,
                                 const TiledWorldDef::RenDef &rendering) {
  TiledWorldDef twd;
  loadWorldDef(twd, node, rendering);
  TiledWorld *world =
      new TiledWorld(node.attribute("tiled").value(),
                     node.attribute("atlas").value(), twd, assets);
  return world;
}

LevelStage::LevelStage(StageManager &manager, AssetManager &assets,
                       const std::string &config,
                       const TiledWorldDef::RenDef &rendering)
    : Stage(manager) {
  auto info = assets.getData(config);
  xml_document doc;
  doc.load_buffer(info->data, info->size);
  auto root = doc.child("stage");
  ui.reset(newTiledWorld(root.child("ui"), assets, rendering));
  level.reset(newTiledWorld(root.child("level"), assets, rendering));
}

void LevelStage::onStart() {
  //
}

void LevelStage::onEnd() {
  //
}

void LevelStage::step(float delta) {
  level->step(delta);
  ui->step(delta);
}

void LevelStage::prepare(bool paused) {
  level->prepare();
  ui->prepare();
}

void LevelStage::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
  target.draw(*level);
  target.draw(*ui);
}

struct FirstQuery : public QueryCallback {
  string name;
  FirstQuery(b2Vec2 p) : QueryCallback(p) {}
  bool callback(const string& name) {
    this->name = name;
    return false;
  }
};

bool LevelStage::onEvent(sf::Event &event) {
  if (event.type == Event::EventType::MouseButtonPressed) {
    if (event.mouseButton.button == Mouse::Left) {
      b2Vec2 pos{static_cast<float32>(event.mouseButton.x),
                 static_cast<float32>(event.mouseButton.y)};
      FirstQuery query(pos);
      ui->query(pos, query);
      if (!query.name.empty()) {
        onClick(query.name);
      }
    }
  }
  return false;
}

void LevelStage::onClick(const std::string &name) {}