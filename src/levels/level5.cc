#ifndef LEVEL5_TITLE_H
#define LEVEL5_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class Level5 : public LevelBase {
private:
  enum {
    NONE, BRIM, OVER,
  } upped;
  float upTime;
protected:
  Level5(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Level5.xml", rendering), upped(NONE), upTime(0) {
    messages.setString("Up");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level5{manager, assets, rendering};
  }

  void step(float delta) override {
    if (state == STARTED) {
      level->getRainDef().rain = true;
      LevelBase::step(delta);
      bool up = level->getPlayer()->GetPosition().y < 0;
      switch (upped) {
      case NONE:
        if (up) {
          upped = BRIM;
        }
        break;
      case BRIM:
        upTime += up ? delta : -delta;
        if (upTime > 1) {
          messages.setString("And that's how\na boat was made (?)");
          upped = OVER;
        }
        break;
      case OVER:
        upTime += up ? delta : -delta;
        if (upTime > 3) {
          manager.unshift("Level6");
          manager.push("Transition");
          manager.erase(this, 1);
          state = RESTARTING;
        }
        break;
      }
    }
  }
};

#endif /* !LEVEL5_TITLE_H */