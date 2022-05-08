#ifndef LEVEL_TITLE_H
#define LEVEL_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level1 : public LevelBase {
private:
  int happyDelay;
protected:
  Level1(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level1.xml", rendering), happyDelay(0) {
    messages.setString("Tutorial");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level1{manager, assets, rendering};
  }

  void step(float delta) override {
    if (state == STARTED) {
      level->getRainDef().rain = true;
    }
    LevelBase::step(delta);
    if (happyDelay > 0) {
      happyDelay--;
      if (happyDelay == 0) {
        manager.unshift("Level2");
        manager.push("Transition");
        manager.erase(this, 1);
        state = RESTARTING;
      }
    }
  }

  void onPlayerMood(PlayerState::Mood mood) override {
    if (state == STARTED) {
      switch (mood) {
      case LevelBase::PlayerState::HAPPY:
        happyDelay = 120;
        break;
      case LevelBase::PlayerState::SAD:
        messages.setString("Failed :(");
        restart();
        break;
      default:
        break;
      }
    }
  }
};

#endif /* !LEVEL_TITLE_H */