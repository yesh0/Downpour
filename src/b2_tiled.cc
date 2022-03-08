#include <string>
#include <vector>

#include "Box2D/Box2D.h"

#include "b2_tiled.h"

using namespace std;

b2Vec2 b2Vec2FromString(string point) {
  auto i = point.find(',');
  if (i != string::npos) {
    return b2Vec2(stof(point), stof(point.substr(i + 1)));
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
        b2Vec2 point = b2Vec2FromString(pointsString.substr(i, next - i));
        vertices.push_back(point * ratio);
        i = next;
      }
      bd.position.Set(x * ratio, y * ratio);
      b2Body *body = world.CreateBody(&bd);
      b2PolygonShape shape;
      shape.Set(vertices.data(), vertices.size());
      body->CreateFixture(&shape, 0);
    }
    if (!object.attribute("width").empty()) {
      int width = stoi(object.attribute("width").value()),
          height = stoi(object.attribute("height").value());
      float hWidth = width * 0.5, hHeight = height * 0.5;
      bd.position.Set((x + hWidth) * ratio, (y + hHeight) * ratio);
      b2Body *body = world.CreateBody(&bd);
      b2PolygonShape shape;
      shape.SetAsBox(hWidth * ratio, hHeight * ratio);
      body->CreateFixture(&shape, 0);
      if (!object.attribute("name").empty()) {
        namedObjects.insert(
            make_pair(string(object.attribute("name").value()), body));
      }
      auto texture =
          object.select_node(".//properties/property[@name='Texture']").node();
      if (!texture.empty()) {
        info.texturedObjects.push_back(make_pair(
            body, B2WorldInfo::TextureInfo{texture.attribute("value").value(),
                                           width, height}));
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
  auto joints = node.find_child_by_attribute("name", "B2Joints");
  if (!joints.empty()) {
    string jointDefs(joints.select_node(".//properties/property[@name='Joints']")
                         .node()
                         .child_value());
    for (int i = 0; i < jointDefs.size(); ++i) {
      auto next = jointDefs.find('\n', i);
      if (next == string::npos) {
        next = jointDefs.size();
      }
      if (i != next) {
        parseJointIntoWorld(jointDefs.substr(i, next - i));
        i = next;
      }
    }
  }
}

const B2WorldInfo &B2Loader::getInfo() const { return info; }

void B2Loader::parseJointIntoWorld(const std::string &jointDef) {}