#include <cstring>
#include <sstream>
#include <string>

#include "SFML/Graphics.hpp"

#include "asset_manager.h"
#define IMPORT_BUILTIN_ASSETS
#include "builtin_assets.cc"

using namespace std;

const AssetInfo *AssetManager::getData(std::string name) const {
  return nullptr;
}

BuiltInAssetManager::BuiltInAssetManager() {
  for (auto i = BUNDLED_ASSETS; i->name != nullptr; ++i) {
    assetMap.insert(std::make_pair(i->name, i));
  }
}

const AssetInfo *BuiltInAssetManager::getData(std::string name) const {
  return assetMap.at(name);
}

bool parseArray(string text, int *array, size_t count) {
  size_t i = 0;
  size_t next = text.find(',');
  do {
    try {
      *array =
          stoi(text.substr(i, next == string::npos ? string::npos : next - i));
    } catch (...) {
      return false;
    }
    i = next == string::npos ? string::npos : next + 1;
    next = text.find(',', i);
    ++array;
    --count;
  } while (i != string::npos && count > 0);
  return true;
}

BundledTexture::BundledTexture(std::string filename, AssetManager *manager)
    : filename(filename), manager(manager) {
  string atlas((const char *)manager->getData(filename)->data);
  stringstream ss(atlas);
  string line;
  string textureFilename("");
  string regionName("");
  int xy[2];
  int size[2];
  int split[4];
  while (std::getline(ss, line).good()) {
    if (line.size() == 0) {
      tryCommitRegionInfo(textureFilename, regionName, xy, size, split);
      /* Marking a new texture file */
      textureFilename.resize(0);
      regionName.resize(0);
    } else if (textureFilename.size() == 0) {
      /* The filename follows the above empty line */
      textureFilename = line;
    } else if (line.find(':') == string::npos) {
      tryCommitRegionInfo(textureFilename, regionName, xy, size, split);
      /* Implying a region name */
      regionName = line;
      memset(xy, 0, sizeof(xy));
      memset(size, 0, sizeof(size));
      memset(split, 0, sizeof(split));
    } else if (line[0] == ' ') {
      /* Properties of a region */
      size_t i = line.find(':');
      string key = line.substr(2, i - 2);
      if (key == "xy") {
        parseArray(line.substr(i + 1), xy, 2);
      } else if (key == "size") {
        parseArray(line.substr(i + 1), size, 2);
      } else if (key == "split") {
        parseArray(line.substr(i + 1), split, 4);
      }
    } else {
      /* Properties of a texture file
       * We are ignoring them for now
       */
    }
  }
}

void BundledTexture::tryCommitRegionInfo(const std::string &textureFilename,
                                         const std::string &regionName,
                                         const int *xy, const int *size,
                                         const int *split) {
  if (textureFilename.size() != 0 && regionName.size() != 0) {
    TextureRegion region{textureFilename,
                         xy[0],
                         xy[1],
                         {size[0], size[1]},
                         {split[0], split[1], split[2], split[3]}};
    textureRegions.insert(make_pair(regionName, region));
  }
}

sf::Sprite BundledTexture::getSprite(std::string name) {
  try {
    const TextureRegion &region = textureRegions.at(name);
    auto i = textures.find(region.filename);
    if (i == textures.end()) {
      auto data = manager->getData(region.filename);
      sf::Texture texture;
      if (texture.loadFromMemory(data->data, data->size)) {
        i = textures.insert(i, make_pair(region.filename, texture));
      } else {
        return sf::Sprite();
      }
    }
    auto &texture = i->second;

    sf::IntRect rect(sf::Vector2i(region.x, region.y),
                     sf::Vector2i(region.size[0], region.size[1]));
    sf::Sprite sprite(texture, rect);
    return sprite;
  } catch (...) {
  }
  return sf::Sprite();
}

NinePatchSprite BundledTexture::getNinePatch(std::string name) {
  getSprite(name);
  const TextureRegion &region = textureRegions.at(name);
  auto &texture = textures.at(region.filename);

  sf::IntRect rect(sf::Vector2i(region.x, region.y),
                   sf::Vector2i(region.size[0], region.size[1]));
  sf::IntRect reg(
      sf::Vector2i(region.split[0], region.split[1]),
      sf::Vector2i(region.size[0] - region.split[0] - region.split[2],
                   region.size[1] - region.split[1] - region.split[3]));
  return NinePatchSprite(texture, rect, reg);
}