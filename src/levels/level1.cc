#ifndef LEVEL_TITLE_H
#define LEVEL_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level1 : public LevelBase {
protected:
  const static LevelBaseDef def;
  Level1(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, def, rendering) {}

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level1{manager, assets, rendering};
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
        manager.unshift("Title");
        manager.erase(this, 1);
        state = RESTARTING;
        break;
      case LevelBase::PlayerState::SAD:
        restart();
        break;
      default:
        break;
      }
    }
  }
};

const LevelBase::LevelBaseDef Level1::def{
    "Level1", "Level1.xml", "TextureBundle.atlas", 20, 4, "plank", "marker1", 1,
    100,      {120, 150}};

#endif /* !LEVEL_TITLE_H */