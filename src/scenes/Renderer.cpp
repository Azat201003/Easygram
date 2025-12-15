#include "Renderer.h"

Component getRenderer(ScreenInteractive &screen, Logger *logger) {
  auto page = std::make_shared<int>(1);
  std::vector<std::shared_ptr<Scene>> scenes;

  scenes.push_back(std::make_shared<LoadingScene>(page, screen, logger));
  scenes.push_back(std::make_shared<PhoneScene>(page, screen, logger));
  scenes.push_back(std::make_shared<CodeScene>(page, screen, logger));
  scenes.push_back(std::make_shared<PasswordScene>(page, screen, logger));
  scenes.push_back(std::make_shared<MainScene>(page, screen, logger));

  std::vector<Component> components;
  for (auto &scene : scenes) {
    components.push_back(scene->getComponent());
  }
  auto container = Container::Tab(components, page.get());

  static std::atomic<bool> running(true);
  static std::thread worker([scenes, page] {
    while (running) {
      TdManager &tdManager = TdManager::getInstance();
      if (tdManager.changeState != TdManager::ChangingState::LOADING &&
          tdManager.authState != TdManager::AuthState::AUTHENTICATED) {
        (*page) = tdManager.authState;
      } else if (tdManager.changeState == TdManager::ChangingState::LOADING) {
        (*page) = 0;
      } else {
        (*page) = 4;
      }
       scenes[*page]->ping();
       std::this_thread::sleep_for(100ms);
    }
  });
  static std::thread updater([] () {
    while (running) {
      TdManager &tdManager = TdManager::getInstance(); 
       tdManager.update_response();
    }
  });

  return CatchEvent(Renderer(container, [scenes, page] {
    return scenes.at(*page)->getElement() | border | center;
  }), [&](Event event) {
        if (event == Event::Character('q')) {
            screen.Exit();
            return true;
        }
        return false;
    });
}
