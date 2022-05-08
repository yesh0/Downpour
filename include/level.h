#ifndef LEVEL_H
#define LEVEL_H

#include <memory>
#include <string>

#include "forward_defs.h"
#include "stage.h"
#include "asset_manager.h"
#include "tiled_world.h"

/**
 * @brief Loads level config, handles basic mouse events
 * 
 * The barebone level consists of two parts: `ui` and `level`.
 */
class LevelStage : public Stage {
protected:
  std::unique_ptr<TiledWorld> ui, level;
  pugi::xml_document doc;
  pugi::xml_node root;
  void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override;

public:
  LevelStage(StageManager &manager, AssetManager &assets,
             const std::string &config, const TiledWorldDef::RenDef &rendering);
  void onStart() override;
  void onEnd() override;
  void step(float delta) override;
  void prepare(bool paused) override;
  bool onEvent(sf::Event &event) override;
  virtual bool onMousedown(B2ObjectInfo &name);
  virtual bool onMouseup(B2ObjectInfo *name);
  virtual bool onHover(sf::Vector2f position);
};

using LevelCreateFunc = LevelStage* (*)(StageManager &manager,
                                        AssetManager &assets,
                                        TiledWorldDef::RenDef &rendering);

struct LevelEntry {
  const char *name;
  LevelCreateFunc create;
};

#endif /* !LEVEL_H */