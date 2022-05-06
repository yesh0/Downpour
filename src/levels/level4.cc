#ifndef LEVEL4_TITLE_H
#define LEVEL4_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level4 : public LevelBase {
protected:
  Level4(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level4.xml", rendering) {
    messages.setString("The Pit (Night)");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level4{manager, assets, rendering};
  }

  void step(float delta) {
    if (state == STARTED) {
      level->getRainDef().rain = true;
    }
    LevelBase::step(delta);
  }

  void onPlayerMood(PlayerState::Mood mood) {
    if (state == STARTED) {
      switch (mood) {
      case LevelBase::PlayerState::HAPPY:
        manager.unshift("Level5");
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

#endif /* !LEVEL4_TITLE_H */