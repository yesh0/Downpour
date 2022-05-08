#ifndef LEVEL2_TITLE_H
#define LEVEL2_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level2 : public LevelBase {
protected:
  Level2(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level2.xml", rendering) {
    messages.setString("The Pit");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level2{manager, assets, rendering};
  }

  void step(float delta) override {
    if (state == STARTED) {
      level->getRainDef().rain = true;
    }
    LevelBase::step(delta);
  }

  void onPlayerMood(PlayerState::Mood mood) override {
    if (state == STARTED) {
      switch (mood) {
      case LevelBase::PlayerState::HAPPY:
        manager.unshift("Level3");
        manager.push("Transition");
        manager.erase(this, 1);
        state = RESTARTING;
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

#endif /* !LEVEL2_TITLE_H */