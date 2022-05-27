#ifndef B2_TILED_H
#define B2_TILED_H

#include <forward_list>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <list>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

#include "pugixml.hpp"

#include "forward_defs.h"

/**
 * @brief World info to be used in TiledWorld
 */
struct B2WorldInfo {
  struct TextureInfo {
    const std::vector<std::string> names;
    const std::map<std::string, std::string> conditionals;
    float w, h;
    bool ninePatched;
    float delay;
    sf::Vector2f offset;
    float rotation;
  };
  std::vector<std::pair<b2Body *, TextureInfo>> texturedObjects;
  std::vector<b2Body *> nodes;
  b2Body *player;
};

/**
 * @brief Object info, b2Body.GetUserData will point to their info (if there is one)
 */
struct B2ObjectInfo {
  const std::string &name;
  b2Body * const body;
  enum Type {
    POLYGON, BOX, CIRCLE, POINT, NODE,
  };
  const Type type;
  TiledWorld *world;
  std::size_t spriteId;
  unsigned int collisionGroup;
};

/**
 * @brief Loads objects in a Tiled file into b2World and extracts their texture info
 *
 * This class handles object loading, if there is extract info to the object,
 * it is put into a B2ObjectInfo and is linked to with b2Body.GetUserData.
 * But since it does not create or render textures, it extracts such info
 * into B2WorldInfo to be used later (in TiledWorld).
 *
 * The format of the Tiled file and property requirements are not documented and I do not
 * plan to. See assets/embedded/...tmx for some examples. You should be able to open them
 * with the Tiled editor: https://github.com/mapeditor/tiled .
 */
class B2Loader {
private:
  B2WorldInfo info;
  b2World &world;
  b2ParticleSystem &particleSystem;
  std::unordered_map<std::string, b2Body *> namedObjects;
  std::forward_list<std::pair<b2Joint *, float>> joints;
  std::forward_list<B2ObjectInfo> objectInfo;
  const float ratio;
  void loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd, std::list<b2Body*> *log);
  void loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd, std::list<b2Body*> *log, bool names);
  std::pair<b2Joint *, float>
  parseJointIntoWorld(const std::string_view &jointDef);

public:
  B2Loader(b2World &world, float ratio, b2ParticleSystem &particleSystem);
  void load(const pugi::xml_node &node);
  B2WorldInfo &getInfo();
  std::forward_list<std::pair<b2Joint *, float>> &getJoints();
  b2Body *findByName(const std::string &name);
  B2ObjectInfo *bindObjectInfo(b2Body *body);

  inline static B2ObjectInfo *getInfo(b2Body *body) {
    return (B2ObjectInfo *)body->GetUserData();
  }
};

#endif /* !B2_TILED_H */
