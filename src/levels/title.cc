#include "level.h"

using namespace std;

class TitleLevel : public LevelStage {
private:
  bool ended;
  TitleLevel(StageManager &manager, AssetManager &assets,
             const TiledWorldDef::RenDef &rendering)
      : LevelStage(manager, assets, "Title.xml", rendering), ended(false) {}

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new TitleLevel{manager, assets, rendering};
  }

  void onClick(B2ObjectInfo &info) {
    if (info.name == "End" && !ended) {
      manager.push("Transition");
      manager.unshift("Level1");
      manager.erase(this, 1);
      ended = true;
    }
  }
};