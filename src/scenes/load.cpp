#include <scenes/load.h>

#include <string>

LoadingScene::LoadingScene(std::shared_ptr<int> page, ScreenInteractive &screen,
                           Logger *logger)
    : Scene(page, screen, logger) {}

Component LoadingScene::getComponent() { return Container::Vertical({}); }

Element LoadingScene::getElement() {
  n++;
  n %= 25;
  return vbox({
      text(" Loading... " + std::string({chars[n]}) + " ") | center,
  });
}
