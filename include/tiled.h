#ifndef DOWNPOUR_TILED_H
#define DOWNPOUR_TILED_H

#include <map>
#include <string>
#include <vector>

#include "SFML/Graphics.hpp"
#include "pugixml.hpp"

#include "asset_manager.h"
#include "b2_tiled.h"

class TiledMap;

struct TiledSet {
  const std::string filename;
  const size_t tileWidth, tileHeight;
  const size_t columns, rows;
};

class TiledLoader {
public:
  using TextureMap = std::unordered_map<std::string, sf::Texture>;
  using TileSetMap = std::map<std::string, TiledSet>;
  using UsedSets = std::map<size_t, TiledSet, std::greater<>>;

private:
  std::unordered_map<std::string, sf::Texture> textures;
  std::map<std::string, TiledSet> tileSets;
  AssetManager &assets;
  void addTileSet(const std::string &name, size_t first, UsedSets &used);

public:
  TiledLoader(AssetManager &manager);
  TiledMap load(std::string filename, B2Loader &b2Loader);
};

class TiledLayer : public sf::Drawable, public sf::Transformable {
private:
  std::string name;
  std::vector<std::vector<sf::Vertex>> vertices;
  std::vector<const sf::Texture *> textures;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;

public:
  TiledLayer(const std::string &name, size_t width, size_t height,
             size_t tileWidth, size_t tileHeight,
             const std::vector<size_t> &tiles,
             const TiledLoader::UsedSets &usedSets,
             const TiledLoader::TextureMap &textureMap);
};

class TiledMap : public sf::Drawable, public sf::Transformable {
  friend class TiledLoader;
private:
  std::vector<TiledLayer> layers;
  TiledMap() = default;
  TiledMap(std::vector<TiledLayer> &&layers);
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;
};

#endif /* !DOWNPOUR_TILED_H */