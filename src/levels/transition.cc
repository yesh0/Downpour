#include "level.h"

using namespace std;

class TransitionStage : public LevelStage {
private:
  float time;
  TransitionStage(StageManager &manager, AssetManager &assets,
             const TiledWorldDef::RenDef &rendering)
      : LevelStage(manager, assets, "Transition.xml", rendering), time(0) {}

public:
  static LevelStage *create(StageManager &manager, AssetManager &assets,
                            const TiledWorldDef::RenDef &rendering) {
    return new TransitionStage{manager, assets, rendering};
  }

  virtual void step(float delta) {
    time += delta;
    if (time > 1) {
      level->getRainDef().rain = false;
      if (time > 2) {
        manager.erase(this);
      }
    }
    LevelStage::step(delta);
  }

  bool onEvent(sf::Event &event) {
    return true;
  }
};