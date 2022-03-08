#ifndef B2_TILED_H
#define B2_TILED_H

#include <map>
#include <vector>
#include <string>

#include "Box2D/Box2D.h"

#include "pugixml.hpp"

struct B2WorldInfo {
  struct TextureInfo {
    const std::string name;
    int w, h;
  };
  std::vector<std::pair<b2Body*, TextureInfo>> texturedObjects;
};

class B2Loader {
private:
  B2WorldInfo info;
  b2World &world;
  std::map<std::string, b2Body*> namedObjects;
  const float ratio;
  void loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd);
  void parseJointIntoWorld(const std::string &jointDef);
public:
  B2Loader(b2World &world, float ratio);
  void load(const pugi::xml_node &node);
  const B2WorldInfo &getInfo() const;
};

#endif /* !B2_TILED_H */