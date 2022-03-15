#ifndef B2_TILED_H
#define B2_TILED_H

#include <map>
#include <vector>
#include <string>
#include <forward_list>
#include <string_view>

#include "Box2D/Box2D.h"

#include "pugixml.hpp"

struct B2WorldInfo {
  struct TextureInfo {
    const std::string name;
    int w, h;
    bool ninePatched;
  };
  std::vector<std::pair<b2Body*, TextureInfo>> texturedObjects;
};

class B2Loader {
private:
  B2WorldInfo info;
  b2World &world;
  std::map<std::string, b2Body*, std::less<>> namedObjects;
  std::forward_list<std::pair<b2Joint*, float>> joints;
  const float ratio;
  void loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd);
  std::pair<b2Joint*, float> parseJointIntoWorld(const std::string_view &jointDef);
public:
  B2Loader(b2World &world, float ratio);
  void load(const pugi::xml_node &node);
  const B2WorldInfo &getInfo() const;
  std::forward_list<std::pair<b2Joint*, float>> &getJoints();
};

#endif /* !B2_TILED_H */