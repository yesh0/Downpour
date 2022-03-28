#include "levels.h"

#include "levels/title.cc"
#include "levels/level1.cc"
#include "levels/transition.cc"

using namespace std;

static LevelConstructorInfo levels[] = {
  { "Title", TitleLevel::create },
  { "Transition", TransitionStage::create },
  { "Level1", Level1::create },
  { nullptr, nullptr },
};

LevelConstructors::LevelConstructors()  {
  for (auto i = levels; i->name != nullptr; ++i) {
    constructors.insert(make_pair(i->name, i));
  }
}

LevelConstructor LevelConstructors::getLevelConstructor(const char *name) {
  return constructors.at(name)->constructor;
}

LevelConstructor LevelConstructors::getLevelConstructor(const string &name) {
  return constructors.at(name)->constructor;
}