#ifndef LEVEL_BASE_H
#define LEVEL_BASE_H

#include <memory>

#include "SFML/Graphics.hpp"

#include "level.h"
#include "nine_patch.h"

class LevelBase : public LevelStage {
public:
  struct LevelBaseDef {
    std::string levelName;
    std::string levelConfig;
    std::string textureBundle;
    float plankDensity;
    float plankWidth;
    std::string plankTexture;
    std::string nodeTexture;
    float jointBreakageForceSq;
    long initialAnger;
    long angerThresholds[2];
  };
  struct Node {
    sf::Vector2f p;
    std::size_t count = 0;
    b2Body *body = nullptr;
    float hw = 0;
  };
  typedef std::list<Node> NodeList;
  struct Plank {
    NinePatchSprite sprite;
    NodeList::iterator start;
    NodeList::iterator end;
  };
  enum State {
    ENDED,
    STARTED,
    RESTARTING,
  };
  struct PlayerState {
    sf::Clock lastHit;
    long anger;
    enum Mood {
      HAPPY, ASLEEP, AWAKE, SAD,
    };
  };

protected:
  State state;
  PlayerState player;
  BundledTexture bundle;
  sf::Font font;
  NinePatchSprite sprite;
  sf::Text messages;
  sf::Vector2f start;
  bool clicked;
  NodeList::iterator startNode;
  NodeList nodes;
  std::vector<Plank> sprites;
  sf::Sprite node;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;
  void bindJoint(Plank &p, b2Body *body, float hw);
  std::pair<b2Body *, float> addBody(Plank &p);
  NodeList::iterator findNode(sf::Vector2f pos, float scale);
  LevelBaseDef def;
  std::list<b2Joint*> joints;

public:
  LevelBase(StageManager &manager, AssetManager &assets,
            const LevelBaseDef &def, const TiledWorldDef::RenDef &rendering);
  void prepare(bool paused);
  void step(float delta);
  bool onEvent(sf::Event &event);
  bool onMousedown(B2ObjectInfo &name);
  bool onHover(sf::Vector2f position);
  void setMessage(const std::string &message);
  void restart();
  virtual void onPlayerMood(PlayerState::Mood mood);
};

#endif /* !LEVEL_BASE_H */