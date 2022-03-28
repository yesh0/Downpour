#ifndef LEVEL_TITLE_H
#define LEVEL_TITLE_H

#include "level.h"

using namespace std;

class Level1 : public LevelStage {
private:
  Level1(StageManager &manager, AssetManager &assets,
             const TiledWorldDef::RenDef &rendering)
      : LevelStage(manager, assets, "Level1.xml", rendering) {}

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new Level1{manager, assets, rendering};
  }

  void onClick(B2ObjectInfo &info) {
  }
};

#endif /* !LEVEL_TITLE_H */