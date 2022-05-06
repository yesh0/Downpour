#include "level_base.h"

using namespace sf;
using namespace std;

const char *LevelBase::PlayerState::MOOD_NAMES[4] = {
    "happy",
    "",
    "awake",
    "sad",
};

LevelBase::LevelBase(StageManager &manager, AssetManager &assets,
                     const std::string &config,
                     const TiledWorldDef::RenDef &rendering)
    : LevelStage(manager, assets, config, rendering), def(loadConfig()),
      bundle(def.textureBundle, assets),
      sprite(bundle.getNinePatch(def.plankTexture)), clicked(false),
      node(bundle.getSprite(def.nodeTexture)), state(ENDED) {
  float scale = level->getRenDef().drawPPM / level->getRenDef().texturePPM;
  sprite.setScale({scale, scale});
  messages.setOutlineColor(Color::Black);
  messages.setFillColor(Color::White);
  messages.setOutlineThickness(3);
  messages.setCharacterSize(48);
  auto f = assets.getData("NimbusRoman-Regular.otf");
  if (!font.loadFromMemory(f->data, f->size)) {
    throw runtime_error("Font load failed");
  }
  messages.setFont(font);
  messages.setPosition({(float)level->getRenDef().screenW - 20,
                        (float)level->getRenDef().screenH - 20});
  node.setScale({scale, scale});
  node.setOrigin(Vector2f(node.getTextureRect().getSize()) * 0.5f);
  for (auto i : level->getNodes()) {
    auto pos = i->GetPosition() * level->getRenDef().drawPPM;
    B2Loader::getInfo(i)->collisionGroup |= 1;
    nodes.push_front({Vector2f{pos.x, pos.y}, 0, i});
  }
  player.anger = def.initialAnger;
  player.mood = PlayerState::ASLEEP;
  player.lastHit.restart();
}

void LevelBase::draw(sf::RenderTarget &target,
                     const sf::RenderStates &states) const {
  LevelStage::draw(target, states);
  if (clicked) {
    target.draw(sprite, states);
  }
  Sprite copy = node;
  for (auto &s : sprites) {
    target.draw(s.sprite, states);
  }
  for (auto &n : nodes) {
    copy.setPosition(n.p);
    target.draw(copy, states);
  }
  target.draw(messages, states);
}

void LevelBase::prepare(bool paused) {
  if (!paused) {
    auto bound = messages.getLocalBounds();
    messages.setOrigin({bound.width, bound.height});
  }
  LevelStage::prepare(paused);
}

LevelBase::NodeList::iterator LevelBase::findNode(sf::Vector2f pos,
                                                  float scale) {
  const float radius = node.getTextureRect().width * 0.5 *
                       level->getRenDef().drawPPM /
                       level->getRenDef().texturePPM * scale;
  const float radiusSq = radius * radius;
  for (auto node = nodes.begin(); node != nodes.end(); ++node) {
    if ((node->p - pos).lengthSq() < radiusSq) {
      return node;
    }
  }
  return nodes.end();
}

bool LevelBase::onEvent(sf::Event &event) {
  if (state == STARTED) {
    return LevelStage::onEvent(event);
  }

  float scale = level->getRenDef().drawPPM / level->getRenDef().texturePPM;
  Vector2f pos;
  const float radius = node.getTextureRect().width * 0.5 * scale;
  NodeList::iterator node;
  switch (event.type) {
  case Event::EventType::MouseButtonPressed:
    pos = {(float)event.mouseButton.x, (float)event.mouseButton.y};
    switch (event.mouseButton.button) {
    case sf::Mouse::Left:
      node = findNode(pos, 1);
      if (node != nodes.end()) {
        /* Marks the starting point */
        start = node->p;
        startNode = node;
        sprite.setPosition(start);
        clicked = true;
        onHover(pos);
        return true;
      } else {
        clicked = false;
        return LevelStage::onEvent(event);
      }
    case sf::Mouse::Right:
      /* Removing right-clicked planks */
      for (auto i = sprites.begin(); i != sprites.end(); ++i) {
        if (i->sprite.getLocalBounds().contains(
                i->sprite.getTransform().getInverse() * pos)) {
          i->start->count--;
          if (i->start->count == 0 && i->start->body == nullptr) {
            nodes.erase(i->start);
          }
          i->end->count--;
          if (i->end->count == 0 && i->end->body == nullptr) {
            nodes.erase(i->end);
          }
          sprites.erase(i);
          return true;
        }
      }
      /* Falling through */
    default:
      clicked = false;
      return LevelStage::onEvent(event);
    }
    break;
  case Event::EventType::MouseButtonReleased:
    if (clicked) {
      /* Creating plank */
      pos = {(float)event.mouseButton.x, (float)event.mouseButton.y};
      if ((start - pos).lengthSq() <=
          def.plankMaxLength * def.plankMaxLength * scale * scale) {
        NodeList::iterator end = findNode(pos, 2);
        if (end == nodes.end()) {
          /* The user does not seem to want to connect to a existing node. */
          nodes.push_front({pos, 0, nullptr});
          end = nodes.begin();
        }
        if (end != startNode) {
          /* onHover adjusts the sprite accordingly */
          onHover(end->p);
          sprites.push_back({sprite, startNode, end});
          startNode->count++;
          end->count++;
        }
      }
      clicked = false;
      return true;
    } else {
      return LevelStage::onEvent(event);
    }
  default:
    return LevelStage::onEvent(event);
  }
}

void LevelBase::step(float delta) {
  if (state == STARTED) {
    for (auto &joint : joints) {
      if (joint != nullptr) {
        if (joint->GetReactionForce(1).LengthSquared() >
            def.jointBreakageForceSq) {
          level->getWorld().DestroyJoint(joint);
          joint = nullptr;
        }
      }
    }
    const int count = level->getParticleSystem().GetBodyContactCount();
    auto contact = level->getParticleSystem().GetBodyContacts();
    for (int i = 0; i != count; ++i, ++contact) {
      auto info = B2Loader::getInfo(contact->body);
      if (info != nullptr) {
        if (info->name == "Player") {
          player.anger++;
          player.lastHit.restart();
        }
      }
    }
    if (player.lastHit.getElapsedTime().asMilliseconds() > 1000) {
      player.anger--;
    }
    auto playerBody = level->getPlayer();
    if (playerBody != nullptr) {
      auto info = B2Loader::getInfo(playerBody);
      PlayerState::Mood next = player.mood;
      if (player.anger < 0) {
        next = PlayerState::HAPPY;
      } else if (player.anger < def.angerThresholds[0]) {
        next = PlayerState::ASLEEP;
      } else if (player.anger < def.angerThresholds[1]) {
        next = PlayerState::AWAKE;
      } else {
        next = PlayerState::SAD;
      }
      if (next != player.mood) {
        info->world->getSprite(info->spriteId)
            ->set(PlayerState::MOOD_NAMES[next]);
        player.mood = next;
        onPlayerMood(next);
      }
    }
  }
  LevelStage::step(delta);
}

pair<b2Body *, float> LevelBase::addBody(Plank &p) {
  b2BodyDef bd;
  float scale = 1 / level->getRenDef().drawPPM;
  auto pos = p.sprite.getPosition() * scale;
  bd.position = {pos.x, pos.y};
  bd.type = b2_dynamicBody;
  bd.angle = p.sprite.getRotation().asRadians();
  auto body = level->getWorld().CreateBody(&bd);
  b2PolygonShape shape;
  float hw = (p.start->p - p.end->p).length() * 0.5 * scale;
  shape.SetAsBox(hw, def.plankWidth * 0.5 / level->getRenDef().texturePPM);
  body->CreateFixture(&shape, def.plankDensity);
  auto sprite = level->bindSprite(body);
  B2WorldInfo::TextureInfo info{
      {}, {}, hw * 2 * level->getRenDef().texturePPM, def.plankWidth, true};
  sprite->push(level->insertByName(info, def.plankTexture, true));
  B2Loader::getInfo(body)->collisionGroup = 0x1;
  return make_pair(body, hw);
}

void LevelBase::bindJoint(Plank &p, b2Body *body, float hw) {
  auto &world = level->getWorld();
  b2DistanceJointDef djf;
  djf.collideConnected = false;
  djf.bodyA = p.start->body;
  djf.bodyB = body;
  djf.length = 1 / level->getRenDef().drawPPM;
  djf.dampingRatio = 0.5;
  djf.localAnchorA.Set(p.start->hw, 0);
  djf.localAnchorB.Set(hw, 0);
  if (djf.bodyA != djf.bodyB) {
    joints.push_back(world.CreateJoint(&djf));
  }
  djf.bodyA = p.end->body;
  djf.localAnchorA.Set(p.end->hw, 0);
  djf.localAnchorB.Set(-hw, 0);
  if (djf.bodyA != djf.bodyB) {
    joints.push_back(world.CreateJoint(&djf));
  }
}

bool LevelBase::onMousedown(B2ObjectInfo &info) {
  if (info.name == "Start") {
    if (state == STARTED) {
      // restart
      info.world->getSprite(info.spriteId)->reset();
      restart();
    } else if (state == ENDED) {
      // start
      info.world->getSprite(info.spriteId)->set("end");
      for (auto &s : sprites) {
        auto p = addBody(s);
        float scale = 1 / level->getRenDef().drawPPM;
        if (s.start->body == nullptr) {
          s.start->body = p.first;
          s.start->hw = p.second;
        }
        if (s.end->body == nullptr) {
          s.end->body = p.first;
          s.end->hw = -p.second;
        }
        bindJoint(s, p.first, p.second);
      }
      nodes.clear();
      sprites.clear();
      state = STARTED;
    }
  }
  return true;
}

void LevelBase::restart() {
  state = RESTARTING;
  manager.push("Transition");
  manager.unshift(def.levelName);
  manager.erase(this, 1);
}

bool LevelBase::onHover(Vector2f position) {
  if (clicked) {
    float scale = level->getRenDef().drawPPM / level->getRenDef().texturePPM;
    auto dist = start - position;
    auto distance = dist.length();
    auto max = def.plankMaxLength * scale;
    if (distance > scale) {
      sprite.setRotation(dist.angle());
    } else {
      sprite.setRotation(radians(0));
      distance = scale * 4;
    }
    if (distance > max) {
      position = start - dist * max / distance;
      distance = max + 0.1;
    }
    auto mid = (start + position) * 0.5f;
    sprite.setPosition(mid);
    Vector2f size{distance / scale, def.plankWidth};
    sprite.setSize(size);
    sprite.setOrigin(size * 0.5f);
    return true;
  }
  return false;
}

void LevelBase::onPlayerMood(PlayerState::Mood mood) {}

LevelBase::LevelBaseDef LevelBase::loadConfig() {
  LevelBaseDef def;
  auto config = root.child("config");
  def.levelName = config.attribute("level").value();
  def.levelConfig = config.attribute("xml").value();
  def.textureBundle = config.attribute("bundle").value();
  auto plank = config.child("plank");
  def.plankDensity = plank.attribute("density").as_float(1);
  def.plankWidth = plank.attribute("width").as_float(4);
  def.plankTexture = plank.attribute("texture").value();
  def.plankMaxLength = plank.attribute("max-length").as_float(32);
  auto node = config.child("node");
  def.nodeTexture = node.attribute("texture").value();
  def.jointBreakageForceSq = config.attribute("joint-breakage").as_float(1);
  auto player = config.child("player");
  def.initialAnger = player.attribute("anger").as_int();
  def.angerThresholds[0] = player.attribute("awake").as_int();
  def.angerThresholds[1] = player.attribute("sad").as_int();
  return def;
}