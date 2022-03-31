#include <cmath>
#include <filesystem>
#include <iostream>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

#include "asset_manager.h"
#include "b2_tiled.h"
#include "level.h"
#include "particle_batch.h"
#include "stage.h"
#include "tiled.h"
#include "tiled_world.h"

using namespace std;
using namespace sf;

int main() {
  RenderWindow window(VideoMode(640, 960),
                      "SFML Works!", Style::Titlebar | Style::Close);
  View view(FloatRect(Vector2f(0, 0), Vector2f(640, 960)));
  window.setView(view);

  TiledWorldDef::RenDef rendering;
  rendering.rainScale = 2.5;
  rendering.texturePPM = 100;
  rendering.drawPPM = 500;
  rendering.screenW = 640;
  rendering.screenH = 960;

#if 0
  BuiltInAssetManager manager;
#else
  FilesystemAssetManager manager("../assets/embedded");
#endif
  BundledTexture bt("TextureBundle.atlas", manager);

  StageManager stageManager(manager, rendering);
  stageManager.push("Title");

  auto s = bt.getNinePatch("steel");
  s.setSize({9, 80});
  s.rotate(sf::degrees(-45));

  size_t frames = 0;
  while (window.isOpen()) {
    ++frames;
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed) {
        window.close();
      } else {
        stageManager.onEvent(event);
      }
    }
    sf::sleep(sf::milliseconds(12));
    window.clear();
    stageManager.step(0.016);
    stageManager.prepare(false);
    window.draw(stageManager);
    window.draw(s);
    window.display();
  }
  return 0;
}
