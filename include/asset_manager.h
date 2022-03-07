#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <map>
#include <unordered_map>
#include <string>

#include "SFML/Graphics.hpp"

#include "nine_patch.h"

struct AssetInfo {
  const char *const name;
  const unsigned char *const data;
  const size_t size;
};

class AssetManager {
public:
  virtual const AssetInfo *getData(std::string name) const;
};

class BuiltInAssetManager : public AssetManager {
private:
  typedef std::map<const std::string, const AssetInfo *> AssetMap;
  AssetMap assetMap;

public:
  BuiltInAssetManager();
  const AssetInfo *getData(std::string name) const;
};

class BundledTexture {
private:
  struct TextureRegion {
    const std::string filename;
    int x, y;
    int size[2];
    int split[4];
  };
  const std::string filename;
  const AssetManager &manager;
  std::unordered_map<std::string, sf::Texture> textures;
  std::map<std::string, TextureRegion> textureRegions;
  void tryCommitRegionInfo(const std::string &textureFilename,
                           const std::string &regionName, const int *xy,
                           const int *size, const int *split);

public:
  BundledTexture(std::string filename, AssetManager &manager);
  sf::Sprite getSprite(std::string name);
  NinePatchSprite getNinePatch(std::string name);
};

#endif /* !ASSET_MANAGER_H */