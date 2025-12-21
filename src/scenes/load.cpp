#include <scenes/load.h>

#include <string>

LoadingScene::LoadingScene(std::shared_ptr<int> page, ScreenInteractive &screen)
    : Scene(page, screen) { }

Component LoadingScene::getComponent() { return Container::Vertical({}); }

Element LoadingScene::getElement() {
  n++;
  n %= 25;
  return vbox({
      text(" Loading... " + std::string({chars[n]}) + " ") | center,
  });
}
