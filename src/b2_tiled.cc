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

void B2Loader::loadIntoWorld(pugi::xml_node &group, b2BodyDef &bd) {
  for (auto object : group.children("object")) {
    int x = stoi(object.attribute("x").value());
    int y = stoi(object.attribute("y").value());
    for (auto poly : object.children("polygon")) {
      string pointsString{poly.attribute("points").value()};
      vector<b2Vec2> vertices;
      for (int i = 0; i < pointsString.size(); ++i) {
        auto next = pointsString.find(' ', i);
        if (next == string::npos) {
          next = pointsString.size();
        }
        b2Vec2 point = b2Vec2FromString(string_view{pointsString}.substr(i, next - i));
        vertices.push_back(point * ratio);
        i = next;
      }
      bd.position.Set(x * ratio, y * ratio);
      b2Body *body = world.CreateBody(&bd);
      b2PolygonShape shape;
      shape.Set(vertices.data(), vertices.size());
      body->CreateFixture(&shape, 1);
    }
    if (!object.attribute("width").empty() || !object.child("point").empty()) {
      b2Body *body;
      if (!object.attribute("width").empty()) {
        int width = stoi(object.attribute("width").value()),
            height = stoi(object.attribute("height").value());
        float hWidth = width * 0.5, hHeight = height * 0.5;
        bd.position.Set((x + hWidth) * ratio, (y + hHeight) * ratio);
        body = world.CreateBody(&bd);
        b2PolygonShape shape;
        shape.SetAsBox(hWidth * ratio, hHeight * ratio);
        body->CreateFixture(&shape, 1);
        auto texture =
            object.select_node(".//properties/property[@name='Texture']")
                .node();
        auto ninePatched =
            object.select_node(".//properties/property[@name='NinePatched']")
                .node();
        if (!texture.empty()) {
          info.texturedObjects.push_back(make_pair(
              body, B2WorldInfo::TextureInfo{texture.attribute("value").value(),
                                             width, height, !ninePatched.empty()}));
        }
      } else {
        bd.position.Set(x * ratio, y * ratio);
        body = world.CreateBody(&bd);
      }
      if (!object.attribute("name").empty()) {
        namedObjects.insert(
            make_pair(string(object.attribute("name").value()), body));
      }
    }
  }
}

B2Loader::B2Loader(b2World &world, float ratio) : world(world), ratio(ratio) {}
void B2Loader::load(const pugi::xml_node &node) {
  for (auto group : node.children("objectgroup")) {
    string type(group.attribute("name").value());
    if (type == "B2Dynamic") {
      b2BodyDef bd;
      bd.type = b2_dynamicBody;
      loadIntoWorld(group, bd);
    } else if (type == "B2Kinematic") {
      b2BodyDef bd;
      bd.type = b2_kinematicBody;
      loadIntoWorld(group, bd);
    } else if (type == "B2Static") {
      b2BodyDef bd;
      bd.type = b2_staticBody;
      loadIntoWorld(group, bd);
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

const B2WorldInfo &B2Loader::getInfo() const { return info; }
std::forward_list<std::pair<b2Joint *, float>> &B2Loader::getJoints() {
  return joints;
}

static void split(const string_view &s, char c, vector<string_view> &output) {
  if (s.empty()) {
    output.push_back(s);
    return;
  }
  for (size_t i = 0; i < s.size(); ++i) {
    size_t next = s.find(c, i);
    if (next == string::npos) {
      next = s.size();
    }
    output.push_back(s.substr(i, next - i));
    i = next;
  }
}

std::pair<b2Joint*, float> B2Loader::parseJointIntoWorld(const std::string_view &jointDef) {
  vector<string_view> args;
  split(jointDef, ' ', args);
  if (args[0] == "Distance") {
    b2DistanceJointDef djd;
    djd.bodyA = namedObjects.find(args[1])->second;
    djd.bodyB = namedObjects.find(args[2])->second;
    djd.length = svtov<float>(args[3]) * ratio;
    djd.localAnchorA.Set(0, 0);
    djd.localAnchorB = b2Vec2FromString(args[4]) * ratio;
    return make_pair(world.CreateJoint(&djd), svtov<float>(args[5]));
  } else if (args[0] == "Rope") {
    b2RopeJointDef rjd;
    rjd.bodyA = namedObjects.find(args[1])->second;
    rjd.bodyB = namedObjects.find(args[2])->second;
    rjd.maxLength = svtov<float>(args[3]) * ratio;
    rjd.localAnchorA.Set(0, 0);
    rjd.localAnchorB = b2Vec2FromString(args[4]) * ratio;
    return make_pair(world.CreateJoint(&rjd), svtov<float>(args[5]));
  } else {
    return make_pair(nullptr, 0);
  }
}