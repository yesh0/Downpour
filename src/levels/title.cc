#include "level.h"

using namespace std;

class TitleLevel : public LevelStage {
private:
  bool ended;
  float time;
  enum {
    DELAYED, RAINING, TRANSITION,
  } transitionState;
  TitleLevel(StageManager &manager, AssetManager &assets,
             const TiledWorldDef::RenDef &rendering)
      : LevelStage(manager, assets, "Title.xml", rendering), ended(false) {}

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new TitleLevel{manager, assets, rendering};
  }

  void step(float delta) override {
    if (ended) {
      time += delta;
      switch (transitionState) {
      case DELAYED:
        if (time > 0.5) {
          transitionState = RAINING;
          level->getRainDef().rain = true;
        }
        break;
      case RAINING:
        if (time > 2.5) {
          transitionState = TRANSITION;
          manager.erase(this, 1);
          manager.push("Transition");
          manager.unshift("Level1");
        }
        break;
      case TRANSITION:
        break;
      }
    }
    LevelStage::step(delta);
  }

  bool rainStep(float delta) override {
    step(delta);
    return transitionState != DELAYED;
  }

  bool onMousedown(B2ObjectInfo &info) override {
    if (info.name == "Start" && !ended) {
      ended = true;
      time = 0;
      transitionState = DELAYED;
      return true;
    } else {
      return false;
    }
  }
};