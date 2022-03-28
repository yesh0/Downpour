#include <algorithm>

#include "levels.h"
#include "stage.h"
#include "level.h"

Stage::Stage(StageManager &manager) : manager(manager) {}

StageManager::StageManager(AssetManager &assets,
                           const TiledWorldDef::RenDef &rendering)
    : Stage(*this), time(0), assets(assets), rendering(rendering) {}

void StageManager::onStart() {}
void StageManager::onEnd() {}

void StageManager::draw(sf::RenderTarget &target,
                        const sf::RenderStates &states) const {
  for (auto &ptr : stages) {
    target.draw(*ptr, states);
  }
}

bool StageManager::onEvent(sf::Event &event) {
  for (auto i = stages.rbegin(); i != stages.rend(); ++i) {
    if ((*i)->onEvent(event)) {
      return true;
    }
  }
  return false;
}

void StageManager::step(float delta) {
  time += delta;
  while (!scheduledRemovals.empty() && scheduledRemovals.top().first <= time) {
    scheduledRemovals.top().second->get()->onEnd();
    stages.erase(scheduledRemovals.top().second);
    scheduledRemovals.pop();
  }
  for (auto &i : stages) {
    i->step(delta);
  }
}

void StageManager::prepare(bool paused) {
  for (auto &i : stages) {
    i->prepare(paused);
  }
}

void StageManager::push(std::unique_ptr<Stage> stage) {
  stages.push_back(std::move(stage));
  stages.rbegin()->get()->onStart();
}

Stage *StageManager::push(const std::string &name) {
  auto raw = levels.getLevelConstructor(name)(*this, assets, rendering);
  std::unique_ptr<Stage> ptr((Stage *)raw);
  push(std::move(ptr));
  return raw;
}

void StageManager::unshift(std::unique_ptr<Stage> stage) {
  stages.push_front(std::move(stage));
  stages.begin()->get()->onStart();
}

Stage *StageManager::unshift(const std::string &name) {
  auto raw = levels.getLevelConstructor(name)(*this, assets, rendering);
  std::unique_ptr<Stage> ptr((Stage *)raw);
  unshift(std::move(ptr));
  return raw;
}

void StageManager::pop() {
  stages.rbegin()->get()->onEnd();
  stages.pop_back();
}

void StageManager::erase(Stage *stage) { erase(stage, 0.01); }

void StageManager::erase(Stage *stage, float when) {
  for (auto i = stages.begin(); i != stages.end(); ++i) {
    if (i->get() == stage) {
      scheduledRemovals.push(std::make_pair(time + when, i));
      return;
    }
  }
}