#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "pugixml.hpp"

#include "b2_tiled.h"
#include "tiled.h"
#include "util.h"

using namespace pugi;
using namespace std;
using namespace sf;

static const TiledLoader::UsedSets::const_iterator
getTiledSetFromTile(size_t tileId, const TiledLoader::UsedSets &usedSets) {
  auto i = usedSets.lower_bound(tileId);
  if (i == usedSets.end()) {
    throw runtime_error("Probably data corruption");
  }
  return i;
}

TiledLayer::TiledLayer(const std::string &name, size_t width, size_t height,
                       size_t tileWidth, size_t tileHeight,
                       const std::vector<size_t> &tiles,
                       const TiledLoader::UsedSets &usedSets,
                       const TiledLoader::TextureMap &textureMap) {
  int i = 0;
  size_t textureCount = 0;
  map<const string, size_t> textureIndices;
  for (int y = 0; y != height; ++y) {
    for (int x = 0; x != width; ++x, ++i) {
      size_t tileId = tiles[i];
      if (tileId == 0) {
        continue;
      }
      /* Get tile info */
      auto tiledSetPair = getTiledSetFromTile(tileId, usedSets);
      auto &tiledSet = tiledSetPair->second;
      size_t tileOffset = tileId - tiledSetPair->first;
      size_t tileSetX = tileOffset % tiledSet.columns;
      size_t tileSetY = tileOffset / tiledSet.columns;

      /* Get corresponding tileSet */
      auto iter = textureIndices.find(tiledSet.filename);
      size_t index;
      if (iter == textureIndices.end()) {
        textureIndices.insert(make_pair(tiledSet.filename, textureCount));
        index = textureCount;
        ++textureCount;
        textures.push_back(&textureMap.at(tiledSet.filename));
        vertices.emplace_back();
      } else {
        index = iter->second;
      }

      /* Compute vertices */
      auto &verts = vertices[index];
      size_t textureW = tiledSet.tileWidth, textureH = tiledSet.tileHeight;
      size_t textureX = textureW * tileSetX, textureY = textureH * tileSetY;
      size_t tileX = tileWidth * x, tileY = tileHeight * y;
      verts.push_back(
          Vertex(Vector2f(tileX, tileY), Vector2f(textureX, textureY)));
      verts.push_back(Vertex(Vector2f(tileX + tileWidth, tileY),
                             Vector2f(textureX + textureW, textureY)));
      verts.push_back(Vertex(Vector2f(tileX, tileY + tileHeight),
                             Vector2f(textureX, textureY + textureH)));
      verts.push_back(Vertex(Vector2f(tileX + tileWidth, tileY),
                             Vector2f(textureX + textureW, textureY)));
      verts.push_back(Vertex(Vector2f(tileX, tileY + tileHeight),
                             Vector2f(textureX, textureY + textureH)));
      verts.push_back(
          Vertex(Vector2f(tileX + tileWidth, tileY + tileHeight),
                 Vector2f(textureX + textureW, textureY + textureH)));
    }
  }
}

void TiledLayer::draw(sf::RenderTarget &target,
                      const sf::RenderStates &states) const {
  RenderStates mine(states);
  mine.transform *= getTransform();
  for (int i = 0; i != textures.size(); ++i) {
    auto &verts = vertices[i];
    auto texture = textures[i];
    mine.texture = texture;
    target.draw(verts.data(), verts.size(), sf::Triangles, mine);
  }
}

TiledLoader::TiledLoader(AssetManager &manager) : assets(manager) {}

static TiledSet loadFromTSX(string filename, AssetManager &assets) {
  auto file = assets.getData(filename);
  xml_document tsx;
  tsx.load_buffer(file->data, file->size);
  auto tilesetNode = tsx.child("tileset");
  auto tileWidth = tilesetNode.attribute("tilewidth").as_uint(),
       tileHeight = tilesetNode.attribute("tileheight").as_uint(),
       columns = tilesetNode.attribute("columns").as_uint(),
       rows = tilesetNode.attribute("tilecount").as_uint() / columns;
  return TiledSet{tilesetNode.child("image").attribute("source").value(),
                  tileWidth, tileHeight, columns, rows};
}

static void parseTileData(const string &data, vector<size_t> &tiles) {
  tiles.clear();
  for (size_t i = 0; i < data.size(); ++i) {
    try {
      size_t offset;
      size_t tile = svtov<size_t>(string_view{data}.substr(i), offset);
      tiles.push_back(tile);
      i += offset;
    } catch (...) {
    }
  }
}

TiledMap TiledLoader::load(std::string filename, B2Loader &b2Loader) {
  auto file = assets.getData(filename);
  xml_document tmx;
  tmx.load_buffer(file->data, file->size);

  auto mapNode = tmx.child("map");
  size_t columns, rows;
  if (string("left-down") == mapNode.attribute("renderorder").value()) {
    /* Map info */
    auto tileWidth = mapNode.attribute("tilewidth").as_uint();
    auto tileHeight = mapNode.attribute("tileheight").as_uint();

    /* Gather tilesets */
    UsedSets usedSets;
    for (auto node : mapNode.children("tileset")) {
      auto tilesetFile = string(node.attribute("source").value());
      addTileSet(tilesetFile, node.attribute("firstgid").as_uint(), usedSets);
    }

    columns = mapNode.attribute("width").as_uint();
    rows = mapNode.attribute("height").as_uint();
    /* Extract layers */
    vector<TiledLayer> layers;
    for (auto node : mapNode.children("layer")) {
      auto id = node.attribute("id").as_uint();
      string name(node.attribute("name").value());
      auto width = node.attribute("width").as_uint();
      auto height = node.attribute("height").as_uint();
      auto dataNode = node.child("data");
      if (string("csv") == dataNode.attribute("encoding").value()) {
        auto data = dataNode.child_value();
        vector<size_t> tiles;
        tiles.reserve(width * height);
        parseTileData(data, tiles);
        layers.emplace_back(name, width, height, tileWidth, tileHeight, tiles,
                            usedSets, textures);
      }
    }
    b2Loader.load(mapNode);
    return TiledMap(std::move(layers));
  } else {
    return TiledMap{};
  }
}

void TiledLoader::addTileSet(const std::string &name, size_t first,
                             UsedSets &used) {
  /* Load tileset if not loaded */
  auto i = tileSets.lower_bound(name);
  if (i == tileSets.end() || i->first != name) {
    i = tileSets.insert(i, make_pair(name, loadFromTSX(name, assets)));
    auto &image = i->second.filename;
    /* Load tile image if not loaded */
    auto j = textures.try_emplace(image);
    if (j.second) {
      auto textureData = assets.getData(image);
      if (!j.first->second.loadFromMemory(textureData->data,
                                          textureData->size)) {
        throw runtime_error("Probably data corruption");
      }
    }
  }
  used.insert(make_pair(first, i->second));
}

TiledMap::TiledMap(std::vector<TiledLayer> &&layers) : layers(layers) {}
void TiledMap::draw(sf::RenderTarget &target,
                    const sf::RenderStates &states) const {
  RenderStates mine(states);
  mine.transform *= getTransform();
  for (auto &layer : layers) {
    target.draw(layer, mine);
  }
}
