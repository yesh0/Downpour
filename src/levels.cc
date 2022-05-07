#include "levels.h"

#include "levels/title.cc"
#include "levels/level1.cc"
#include "levels/level2.cc"
#include "levels/level3.cc"
#include "levels/level4.cc"
#include "levels/level5.cc"
#include "levels/level6.cc"
#include "levels/level7.cc"
#include "levels/thank.cc"
#include "levels/transition.cc"

using namespace std;

static LevelConstructorInfo levels[] = {
  { "Title", TitleLevel::create },
  { "Transition", TransitionStage::create },
  { "Level1", Level1::create },
  { "Level2", Level2::create },
  { "Level3", Level3::create },
  { "Level4", Level4::create },
  { "Level5", Level5::create },
  { "Level6", Level6::create },
  { "Level7", Level7::create },
  { "Thank", LevelThank::create },
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