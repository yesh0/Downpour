#ifndef LEVEL6_TITLE_H
#define LEVEL6_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level6 : public LevelBase {
private:
  enum {
    NONE, SCROLLING,
  } scroll;
  sf::Clock scrollClock;
protected:
  Level6(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level6.xml", rendering), scroll(NONE) {
    messages.setString("Up Up");
  }
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override {
    // Had to hardcode this
    target.clear(sf::Color(0x91d3ffff));
    LevelBase::draw(target, states);
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level6{manager, assets, rendering};
  }

  void step(float delta) override {
    if (state == STARTED) {
      level->getRainDef().rain = true;
      LevelBase::step(delta * 2);
      switch (scroll) {
      case NONE:
        scroll = SCROLLING;
        scrollClock.restart();
        break;
      case SCROLLING:
        auto &rendering = level->getRenDef();
        auto height = scrollClock.getElapsedTime().asSeconds() * rendering.drawPPM / 2;
        level->setPosition({
          0,
          height
        });
        float y = level->getPlayer()->GetPosition().y;
        if (y * rendering.drawPPM + height > rendering.screenH) {
          restart();
        } else if (y < -1) {
          manager.unshift("Level7");
          manager.push("Transition");
          manager.erase(this, 1);
          state = RESTARTING;
        }
        break;
      }
    }
  }
};

#endif /* !LEVEL6_TITLE_H */