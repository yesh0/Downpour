#ifndef LEVEL8_TITLE_H
#define LEVEL8_TITLE_H

#include "level.h"
#include "level_base.h"

using namespace std;
using namespace sf;

class LevelThank : public LevelBase {
private:
protected:
  LevelThank(StageManager &manager, AssetManager &assets,
         const TiledWorldDef::RenDef &rendering)
      : LevelBase(manager, assets, "Thank.xml", rendering) {
    messages.setString("Thank you\nfor playing!");
  }

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new LevelThank{manager, assets, rendering};
  }
};

#endif /* !LEVEL_THANK_TITLE_H */