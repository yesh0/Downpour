#ifndef STAGE_H
#define STAGE_H

#include <list>
#include <memory>
#include <queue>
#include <utility>

#include "SFML/Graphics.hpp"

class StageManager;

class Stage : public sf::Drawable {
protected:
  StageManager &manager;
  Stage(StageManager &manager);
  virtual void draw(sf::RenderTarget &target,
                    const sf::RenderStates &states) const = 0;

public:
  virtual ~Stage() = default;
  virtual void onStart() = 0;
  virtual void onEnd() = 0;
  virtual void step(float delta) = 0;
  /**
   * @brief A workaround since sf::Drawable::draw is marked const
   *
   * @param paused
   */
  virtual void prepare(bool paused) = 0;
  virtual bool onEvent(sf::Event &event) = 0;
};

class StageManager : public Stage {
private:
  typedef std::list<std::unique_ptr<Stage>> Stages;
  typedef std::pair<float, Stages::iterator> TimedStage;
  struct TimedStageLess
      : public std::binary_function<TimedStage, TimedStage, bool> {
    bool operator()(const TimedStage &a, const TimedStage &b) {
      return a.first < b.first;
    }
  };
  Stages stages;
  float time;
  std::less<int> l;
  std::priority_queue<TimedStage, std::vector<TimedStage>, TimedStageLess>
      scheduledRemovals;

protected:
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;

public:
  StageManager();
  void onStart();
  void onEnd();
  void step(float delta);
  void prepare(bool paused);
  bool onEvent(sf::Event &event);
  void push(std::unique_ptr<Stage> stage);
  void pop();
  void erase(Stage *stage);
  void erase(Stage *stage, float when);
};

#endif /* !STAGE_H */