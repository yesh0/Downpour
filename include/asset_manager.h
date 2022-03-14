#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <filesystem>
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
  virtual const AssetInfo *getData(const std::string &name);
};

class BuiltInAssetManager : public AssetManager {
private:
  typedef std::map<const std::string, const AssetInfo *> AssetMap;
  AssetMap assetMap;

public:
  BuiltInAssetManager();
  const AssetInfo *getData(const std::string &name);
};

/**
 * @brief Testing purpose file provider
 *
 * All file contents are cached
 */
class FilesystemAssetManager : public AssetManager {
private:
  static void deleteInfo(AssetInfo *info);
  typedef std::unique_ptr<AssetInfo, decltype(&deleteInfo)> Ptr;
  typedef std::map<const std::string,
                   Ptr>
      AssetMap;
  AssetMap assetMap;
  std::filesystem::path dir;

public:
  FilesystemAssetManager(const std::string &assetDir);
  const AssetInfo *getData(const std::string &name);
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
  AssetManager &manager;
  std::unordered_map<std::string, sf::Texture> textures;
  std::map<std::string, TextureRegion> textureRegions;
  void tryCommitRegionInfo(const std::string &textureFilename,
                           const std::string &regionName, const int *xy,
                           const int *size, const int *split);

public:
  BundledTexture(const std::string &filename, AssetManager &manager);
  sf::Sprite getSprite(const std::string &name);
  NinePatchSprite getNinePatch(const std::string &name);
};

#endif /* !ASSET_MANAGER_H */