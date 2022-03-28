#ifndef LEVELS_H
#define LEVELS_H

#include <map>
#include <string>

#include "forward_defs.h"
#include "tiled_world.h"

typedef LevelStage *(*LevelConstructor)(StageManager &manager,
                                        AssetManager &assets,
                                        const TiledWorldDef::RenDef &rendering);

struct LevelConstructorInfo {
  const char *name;
  const LevelConstructor constructor;
};

class LevelConstructors {
private:
  std::map<std::string, LevelConstructorInfo*> constructors;
public:
  LevelConstructors();
  LevelConstructor getLevelConstructor(const char *name);
  LevelConstructor getLevelConstructor(const std::string &name);
};

#endif /* !LEVELS_H */