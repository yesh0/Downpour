#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <filesystem>
#include <map>
#include <unordered_map>
#include <string>

#include "SFML/Graphics.hpp"

#include "nine_patch.h"

/**
 * @brief Semantically a "file"
 */
struct AssetInfo {
  const char *const name;
  const unsigned char *const data;
  const size_t size;
};

/**
 * @brief An interface for asset R/W
 */
class AssetManager {
public:
  virtual const AssetInfo *getData(const std::string &name) = 0;
};

/**
 * @brief An "AssetManager" to be used with bundled resources
 *
 * See assets/file_embedder.cc for how files are embedded.
 * It is supposed to be used when releasing into a single executable.
 */
class BuiltInAssetManager : public AssetManager {
private:
  using AssetMap = std::map<const std::string, const AssetInfo *> ;
  AssetMap assetMap;

public:
  BuiltInAssetManager();
  const AssetInfo *getData(const std::string &name) override;
};

/**
 * @brief Testing purpose file provider
 *
 * All file contents are cached
 */
class FilesystemAssetManager : public AssetManager {
private:
  static void deleteInfo(AssetInfo *info);
  using Ptr = std::unique_ptr<AssetInfo, decltype(&deleteInfo)>;
  using AssetMap = std::map<const std::string, Ptr>;
  AssetMap assetMap;
  std::filesystem::path dir;

public:
  FilesystemAssetManager(const std::string &assetDir);
  const AssetInfo *getData(const std::string &name);
};

/**
 * @brief Extracts separate textures from texture bundles
 * 
 * Textures are bundled using GDX Texture Packer, that is, many images are gathered together
 * into a single large image.
 *
 * This class loads the "atlas" info for the larger image, and extracts portions of the larger
 * image into the needed real textures.
 *
 * For example, you may open assets/Downpour.tpproj with:
 * https://github.com/crashinvaders/gdx-texture-packer-gui/
 * for an overview.
 */
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
  /**
   * @brief Construct a new Bundled Texture object
   * 
   * @param filename the .atlas filename
   * @param manager the AssetManager
   */
  BundledTexture(const std::string &filename, AssetManager &manager);
  /**
   * @brief Get the Sprite object
   * 
   * @param name the name of the texture in the .atlas file
   * @return sf::Sprite the sprite
   */
  sf::Sprite getSprite(const std::string &name);
  /**
   * @brief Get the Nine Patch object
   * 
   * @param name the name of the texture in the .atlas file
   * @return NinePatchSprite see NinePatchSprite
   */
  NinePatchSprite getNinePatch(const std::string &name);
};

#endif /* !ASSET_MANAGER_H */