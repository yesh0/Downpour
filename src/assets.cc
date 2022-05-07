#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "SFML/Graphics.hpp"

#include "asset_manager.h"
#include "util.h"
#define IMPORT_BUILTIN_ASSETS
#include "builtin_assets.cc"

using namespace std;

const AssetInfo *AssetManager::getData(const std::string &name) {
  return nullptr;
}

BuiltInAssetManager::BuiltInAssetManager() {
  for (auto i = BUNDLED_ASSETS; i->name != nullptr; ++i) {
    assetMap.insert(std::make_pair(i->name, i));
  }
}

const AssetInfo *BuiltInAssetManager::getData(const std::string &name) {
  return assetMap.at(name);
}

void FilesystemAssetManager::deleteInfo(AssetInfo *info) {
  delete[] info->data;
  delete[] info->name;
  delete info;
}

FilesystemAssetManager::FilesystemAssetManager(const std::string &assetDir)
    : dir(assetDir) {}

const AssetInfo *FilesystemAssetManager::getData(const std::string &name) {
  auto i = assetMap.find(name);
  if (i == assetMap.end()) {
    auto file{dir};
    file.append(name);
    auto nameAlloc = new char[name.size() + 1];
    nameAlloc[name.copy(nameAlloc, name.size())] = 0;
    auto size = filesystem::file_size(file);
    auto data = new unsigned char[size];
    ifstream f(file);
    f.read((char *)data, size);
    f.close();
    auto info = new AssetInfo{nameAlloc, data, size};
    assetMap.insert(make_pair(name, Ptr{info, &deleteInfo}));
    return info;
  } else {
    return i->second.get();
  }
}

bool parseArray(const string_view &text, int *array, size_t count) {
  size_t i = 0;
  size_t next = text.find(',');
  do {
    try {
      *array = svtov<int>(string_view{text}.substr(
          i, next == string::npos ? string::npos : next - i));
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

BundledTexture::BundledTexture(const std::string &filename,
                               AssetManager &manager)
    : filename(filename), manager(manager) {
  auto data = manager.getData(filename);
  string atlas((const char *)data->data, data->size);
  stringstream ss(atlas);
  string line;
  string textureFilename("");
  string regionName("");
  int xy[2];
  int size[2];
  int split[4];
  while (std::getline(ss, line)) {
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
      string_view l{line};
      string_view key = l.substr(2, i - 2);
      if (key == "xy") {
        if (!parseArray(l.substr(i + 1), xy, 2)) {
          throw runtime_error("Internal file corruption");
        }
      } else if (key == "size") {
        if (!parseArray(l.substr(i + 1), size, 2)) {
          throw runtime_error("Internal file corruption");
        }
      } else if (key == "split") {
        if (!parseArray(l.substr(i + 1), split, 4)) {
          throw runtime_error("Internal file corruption");
        }
      }
    } else {
      /* Properties of a texture file
       * We are ignoring them for now
       */
    }
  }
  tryCommitRegionInfo(textureFilename, regionName, xy, size, split);
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

sf::Sprite BundledTexture::getSprite(const std::string &name) {
  try {
    const TextureRegion &region = textureRegions.at(name);
    auto pair = textures.try_emplace(region.filename);
    if (pair.second) {
      auto data = manager.getData(region.filename);
      if (!pair.first->second.loadFromMemory(data->data, data->size)) {
        textures.erase(pair.first);
        return sf::Sprite();
      }
    }
    auto &texture = pair.first->second;

    sf::IntRect rect(sf::Vector2i(region.x, region.y),
                     sf::Vector2i(region.size[0], region.size[1]));
    sf::Sprite sprite(texture, rect);
    return sprite;
  } catch (...) {
  }
  return sf::Sprite();
}

NinePatchSprite BundledTexture::getNinePatch(const std::string &name) {
  getSprite(name);
  const TextureRegion &region = textureRegions.at(name);
  auto &texture = textures.at(region.filename);

  sf::IntRect rect(sf::Vector2i(region.x, region.y),
                   sf::Vector2i(region.size[0], region.size[1]));
  sf::IntRect reg(
      sf::Vector2i(region.split[0], region.split[2]),
      sf::Vector2i(region.size[0] - region.split[0] - region.split[1],
                   region.size[1] - region.split[2] - region.split[3]));
  return NinePatchSprite(texture, rect, reg);
}