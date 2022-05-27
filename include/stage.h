#ifndef STAGE_H
#define STAGE_H

#include <list>
#include <memory>
#include <queue>
#include <utility>

#include "SFML/Graphics.hpp"

#include "forward_defs.h"
#include "asset_manager.h"
#include "rain_mixer.h"
#include "tiled_world.h"
#include "levels.h"

/**
 * @brief Semantically a stage
 */
class Stage : public sf::Drawable {
protected:
  StageManager &manager;
  Stage(StageManager &manager);
  void draw(sf::RenderTarget &target,
            const sf::RenderStates &states) const override = 0;

public:
  ~Stage() override = default;
  virtual void onStart() = 0;
  virtual void onEnd() = 0;
  virtual void step(float delta) = 0;
  /**
   * @brief step() and returns whether the stage is raining
   * 
   * @param delta as is in delta
   * @return true when raining
   * @return false otherwise
   */
  virtual bool rainStep(float delta) {
    step(delta);
    return false;
  }
  /**
   * @brief A workaround since sf::Drawable::draw is marked const
   *
   * @param paused
   */
  virtual void prepare(bool paused) = 0;
  virtual bool onEvent(sf::Event &event) = 0;
};

/**
 * @brief Manages all stages displayed, plays downpour sound when any of them are raining
 *
 * Stages are layered.
 */
class StageManager : public Stage {
private:
  using Stages = std::list<std::unique_ptr<Stage>>;
  using TimedStage = std::pair<float, Stages::iterator>;
  struct TimedStageLess {
    bool operator()(const TimedStage &a, const TimedStage &b) {
      return a.first < b.first;
    }
  };

  Stages stages;
  AssetManager& assets;
  const TiledWorldDef::RenDef& rendering;
  float time;
  LevelConstructors levels;
  std::less<int> l;
  std::priority_queue<TimedStage, std::vector<TimedStage>, TimedStageLess>
      scheduledRemovals;
  RainMixer &mixer;

protected:
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;
  void push(std::unique_ptr<Stage> stage);
  void unshift(std::unique_ptr<Stage> stage);

public:
  StageManager(AssetManager&, RainMixer &mixer, const TiledWorldDef::RenDef&);
  void onStart() override;
  void onEnd() override;
  void step(float delta) override;
  void prepare(bool paused) override;
  bool onEvent(sf::Event &event) override;

  /**
   * @brief Creates the stage of "name" and pushes it to the front
   * 
   * @param name 
   * @return Stage* 
   */
  Stage *push(const std::string &name);
  /**
   * @brief Pushes the stage to the back
   * 
   * @param stage 
   */
  /**
   * @brief Creates the stage of "name" and pushes it to the back
   * 
   * @param name
   * @return Stage* 
   */
  Stage *unshift(const std::string &name);

  void erase(Stage *stage);
  /**
   * @brief Erase a stage after some time
   * 
   * @param stage the stage
   * @param when delay in seconds
   */
  void erase(Stage *stage, float when);
};

#endif /* !STAGE_H */