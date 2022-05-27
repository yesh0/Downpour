#include <cmath>
#include <filesystem>
#include <iostream>

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

#include "asset_manager.h"
#include "b2_tiled.h"
#include "level.h"
#include "particle_batch.h"
#include "rain_mixer.h"
#include "rate_limiter.h"
#include "stage.h"
#include "tiled.h"
#include "tiled_world.h"

using namespace std;
using namespace sf;

int main(int argc, char** argv) {
  RenderWindow window(VideoMode(640, 960),
                      "Downpour - The Game", Style::Titlebar | Style::Close);
  window.setPosition({0, 0});
  View view(FloatRect(Vector2f(0, 0), Vector2f(640, 960)));
  window.setView(view);

  TiledWorldDef::RenDef rendering;
  rendering.rainScale = 2.5;
  rendering.texturePPM = 10;
  rendering.drawPPM = 50;
  rendering.screenW = 640;
  rendering.screenH = 960;

#ifdef BUNDLE_DOWNPOUR
  BuiltInAssetManager manager;
#else
  FilesystemAssetManager manager("../assets/embedded");
#endif

  auto iconData = manager.getData("downpour-dirt.png");
  Image icon;
  if (icon.loadFromMemory(iconData->data, iconData->size)) {
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
  }

  RainMixer mixer(manager, 1, 21, "waterfall.ogg");

  StageManager stageManager(manager, mixer, rendering);
#ifdef BUNDLE_DOWNPOUR
  /* Release */
  stageManager.push("Title");
#else
  /* Debug */
  if (argc == 1) {
    stageManager.push("Title");
  } else {
    stageManager.push(argv[1]);
  }
#endif

  RateLimiter limiter{60};
  float delta = 1. / 60;
  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed) {
        window.close();
      } else {
        stageManager.onEvent(event);
      }
    }

    window.clear();
    stageManager.step(delta);
    stageManager.prepare(false);
    window.draw(stageManager);
    window.display();

    mixer.drop();
    delta = limiter();
  }

  window.close();
  return 0;
}
