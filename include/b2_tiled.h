#ifndef B2_TILED_H
#define B2_TILED_H

#include <forward_list>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

#include "pugixml.hpp"

struct B2WorldInfo {
  struct TextureInfo {
    const std::vector<std::string> names;
    const std::map<std::string, std::string> conditionals;
    int w, h;
    bool ninePatched;
    float delay;
  };
  std::vector<std::pair<b2Body *, TextureInfo>> texturedObjects;
};

struct B2ObjectInfo {
  const std::string &name;
  const b2Body *body;
  enum Type {
    POLYGON, BOX, CIRCLE, POINT, NODE,
  };
  const Type type;
  sf::Sprite *sprite;
};

class B2Loader {
private:
  B2WorldInfo info;
  b2World &world;
  std::unordered_map<std::string, b2Body *> namedObjects;
  std::forward_list<std::pair<b2Joint *, float>> joints;
  std::forward_list<B2ObjectInfo> objectInfo;
  const float ratio;
  void loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd);
  std::pair<b2Joint *, float>
  parseJointIntoWorld(const std::string_view &jointDef);

public:
  B2Loader(b2World &world, float ratio);
  void load(const pugi::xml_node &node);
  const B2WorldInfo &getInfo() const;
  std::forward_list<std::pair<b2Joint *, float>> &getJoints();
  b2Body *findByName(const std::string &name);

  inline static B2ObjectInfo *getInfo(b2Body *body) {
    return (B2ObjectInfo *)body->GetUserData();
  }
};

#endif /* !B2_TILED_H */
