#include <string>
#include <string_view>
#include <vector>

#include "Box2D/Box2D.h"

#include "b2_tiled.h"
#include "util.h"

using namespace std;

b2Vec2 b2Vec2FromString(const string_view &point) {
  auto i = point.find(',');
  if (i != string::npos) {
    return b2Vec2(svtov<float>(point), svtov<float>(point.substr(i + 1)));
  } else {
    return b2Vec2();
  }
}

void B2Loader::loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd,
                             std::list<b2Body *> *log) {
  for (auto object : group.children("object")) {
    b2Body *body = nullptr;
    auto name = object.attribute("name");
    b2Vec2 offset{0, 0};
    bool objectExists = false;
    if (name) {
      auto bodyI = namedObjects.find(name.value());
      if (bodyI != namedObjects.end()) {
        body = bodyI->second;
        offset = -body->GetPosition();
        objectExists = true;
      }
    }

    int x = object.attribute("x").as_int();
    int y = object.attribute("y").as_int();
    B2ObjectInfo::Type type;
    if (object.child("polygon")) {
      type = B2ObjectInfo::POLYGON;
      auto poly = object.child("polygon");
      if (body == nullptr) {
        bd.position.Set(x * ratio, y * ratio);
        body = world.CreateBody(&bd);
      } else {
        offset += {x * ratio, y * ratio};
      }
      string pointsString{poly.attribute("points").value()};
      vector<b2Vec2> vertices;
      for (int i = 0; i < pointsString.size(); ++i) {
        auto next = pointsString.find(' ', i);
        if (next == string::npos) {
          next = pointsString.size();
        }
        b2Vec2 point =
            b2Vec2FromString(string_view{pointsString}.substr(i, next - i));
        vertices.push_back(point * ratio + offset);
        i = next;
      }
      b2PolygonShape shape;
      shape.Set(vertices.data(), vertices.size());
      body->CreateFixture(&shape, 1);
      if (log != nullptr) {
        log->push_back(body);
      }
    } else if (object.attribute("width") || object.child("point")) {
      auto textures =
          object.select_node(".//properties/property[@name='Textures']").node();
      auto ellipse = object.select_node(".//ellipse");
      if (object.attribute("width") || textures) {
        int width = object.attribute("width").as_int(0),
            height = object.attribute("height").as_int(0);
        float hWidth = width * 0.5, hHeight = height * 0.5;
        if (body == nullptr) {
          bd.position.Set((x + hWidth) * ratio, (y + hHeight) * ratio);
          body = world.CreateBody(&bd);
        } else {
          offset += {(x + hWidth) * ratio, (y + hHeight) * ratio};
        }
        float density =
            object.select_node(".//properties/property[@name='Density']")
                .node()
                .attribute("value")
                .as_float(1);
        if (object.child("point")) {
          b2CircleShape shape;
          shape.m_p = offset;
          shape.m_radius = 3 * ratio;
          body->CreateFixture(&shape, density);
          width = height = 6;
          type = B2ObjectInfo::Type::NODE;
          info.nodes.push_back(body);
        } else if (ellipse) {
          b2CircleShape shape;
          shape.m_p = offset;
          shape.m_radius = hWidth * ratio;
          body->CreateFixture(&shape, density);
          type = B2ObjectInfo::Type::CIRCLE;
        } else {
          b2PolygonShape shape;
          float hwr = hWidth * ratio, hhr = hHeight * ratio;
          b2Vec2 vertices[4] = {
            b2Vec2{-hwr, -hhr} + offset,
            b2Vec2{hwr, -hhr} + offset,
            b2Vec2{hwr, hhr} + offset,
            b2Vec2{-hwr, hhr} + offset,
          };
          shape.Set(vertices, 4);
          body->CreateFixture(&shape, density);
          type = B2ObjectInfo::Type::BOX;
        }
        auto textures =
            object.select_node(".//properties/property[@name='Textures']")
                .node();
        auto textureCount =
            object.select_node(".//properties/property[@name='TextureCount']")
                .node()
                .attribute("value")
                .as_int();
        auto ninePatched =
            object.select_node(".//properties/property[@name='NinePatched']")
                .node();
        auto delay =
            object.select_node(".//properties/property[@name='TextureDelay']")
                .node();
        if (textureCount > 0) {
          vector<string_view> lines;
          vector<string> names;
          map<string, string> conditionals;
          auto s = textures.attribute("value").empty()
                       ? textures.child_value()
                       : textures.attribute("value").value();
          string_view sv{s};
          split<>(sv, '\n', textureCount, lines);
          for (auto &line : lines) {
            auto sep = line.find('=');
            if (sep == string_view::npos) {
              names.emplace_back(line);
            } else {
              conditionals.insert(
                  make_pair(line.substr(0, sep), line.substr(sep + 1)));
            }
          }
          info.texturedObjects.push_back(make_pair(
              body,
              B2WorldInfo::TextureInfo{names, conditionals, (float)width,
                                       (float)height, !ninePatched.empty(),
                                       delay.attribute("value").as_float(1),
                                       {offset.x / ratio, offset.y / ratio}}));
        }
      } else {
        bd.position.Set(x * ratio, y * ratio);
        body = world.CreateBody(&bd);
        type = B2ObjectInfo::Type::POINT;
      }
    }
    if (!object.attribute("name").empty()) {
      auto i = namedObjects.insert(
          make_pair(string(object.attribute("name").value()), body));
      auto info = objectInfo.insert_after(
          objectInfo.before_begin(), B2ObjectInfo{i.first->first, body, type});
      body->SetUserData((void *)&(*info));
    }
    if (body != nullptr && log != nullptr) {
      log->push_back(body);
    }
  }
}

B2Loader::B2Loader(b2World &world, float ratio) : world(world), ratio(ratio) {}
void B2Loader::load(const pugi::xml_node &node) {
  b2BodyDef bd;
  info.player = nullptr;
  for (auto group : node.children("objectgroup")) {
    string type(group.attribute("name").value());
    if (type == "B2Dynamic") {
      bd.type = b2_dynamicBody;
      loadIntoWorld(group, bd, nullptr);
    } else if (type == "B2Kinematic") {
      bd.type = b2_kinematicBody;
      loadIntoWorld(group, bd, nullptr);
    } else if (type == "B2Static") {
      bd.type = b2_staticBody;
      loadIntoWorld(group, bd, nullptr);
    } else if (type == "Player") {
      list<b2Body *> players;
      bd.type = b2_dynamicBody;
      loadIntoWorld(group, bd, &players);
      if (players.size() >= 1) {
        info.player = players.front();
      }
    }
  }
  auto jointsProperty = node.find_child_by_attribute("name", "B2Joints");
  if (!jointsProperty.empty()) {
    auto property =
        jointsProperty.select_node(".//properties/property[@name='Joints']")
            .node();
    string_view jointDefs(property.attribute("value").empty()
                              ? property.child_value()
                              : property.attribute("value").value());
    for (int i = 0; i < jointDefs.size(); ++i) {
      auto next = jointDefs.find('\n', i);
      if (next == string::npos) {
        next = jointDefs.size();
      }
      if (i != next) {
        joints.insert_after(joints.before_begin(),
                            parseJointIntoWorld(jointDefs.substr(i, next - i)));
        i = next;
      }
    }
  }
}

B2WorldInfo &B2Loader::getInfo() { return info; }
std::forward_list<std::pair<b2Joint *, float>> &B2Loader::getJoints() {
  return joints;
}

std::pair<b2Joint *, float>
B2Loader::parseJointIntoWorld(const std::string_view &jointDef) {
  vector<string_view> args;
  split(jointDef, ' ', args);
  if (args[0] == "Distance") {
    b2DistanceJointDef djd;
    djd.bodyA = namedObjects.find(string{args[1]})->second;
    djd.bodyB = namedObjects.find(string{args[2]})->second;
    djd.length = svtov<float>(args[3]) * ratio;
    djd.localAnchorA.Set(0, 0);
    djd.localAnchorB = b2Vec2FromString(args[4]) * ratio;
    return make_pair(world.CreateJoint(&djd), svtov<float>(args[5]));
  } else if (args[0] == "Rope") {
    b2RopeJointDef rjd;
    rjd.bodyA = namedObjects.find(string{args[1]})->second;
    rjd.bodyB = namedObjects.find(string{args[2]})->second;
    rjd.maxLength = svtov<float>(args[3]) * ratio;
    rjd.localAnchorA.Set(0, 0);
    rjd.localAnchorB = b2Vec2FromString(args[4]) * ratio;
    return make_pair(world.CreateJoint(&rjd), svtov<float>(args[5]));
  } else {
    return make_pair(nullptr, 0);
  }
}

b2Body *B2Loader::findByName(const std::string &name) {
  auto i = namedObjects.find(name);
  if (i == namedObjects.end()) {
    return nullptr;
  } else {
    return i->second;
  }
}

const static std::string placeholder = "_";
B2ObjectInfo *B2Loader::bindObjectInfo(b2Body *body) {
  objectInfo.push_front(
      {placeholder, body, B2ObjectInfo::Type::BOX, nullptr, 0});
  body->SetUserData(&objectInfo.front());
  return &objectInfo.front();
}
