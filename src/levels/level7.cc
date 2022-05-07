#ifndef LEVEL7_TITLE_H
#define LEVEL7_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level7 : public LevelBase {
private:
  bool ended;
  int delay;
protected:
  Level7(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level7.xml", rendering), ended(false), delay(0) {
    messages.setString("Plug");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level7{manager, assets, rendering};
  }

  void step(float delta) override {
    if (state == STARTED) {
      level->getRainDef().rain = true;
      LevelBase::step(delta);
    }
  }

  void onPlayerMood(PlayerState::Mood mood) override {
    if (state == STARTED) {
      switch (mood) {
      case LevelBase::PlayerState::SAD:
        restart();
        break;
      case LevelBase::PlayerState::HAPPY:
        ended = true;
        delay = 0;
        break;
      default:
        break;
      }
    }
    if (ended) {
      if (delay > 0) {
        delay++;
        if (delay > 100) {
          manager.unshift("Thank");
          manager.push("Transition");
          delay = -1;
        }
      }
    }
  }
};

#endif /* !LEVEL7_TITLE_H */