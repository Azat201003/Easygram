#include <renderer.h>
#include <state.h>
#include <telegram/facade.h>
#include <utils/chats.h>
#include <logger/logger.h>
#include <string>

Component getRenderer(ScreenInteractive &screen) {
  auto page = std::make_shared<int>(1);
  std::vector<std::shared_ptr<Scene>> scenes;

  scenes.push_back(std::make_shared<LoadingScene>(page, screen));
  scenes.push_back(std::make_shared<PhoneScene>(page, screen));
  scenes.push_back(std::make_shared<CodeScene>(page, screen));
  scenes.push_back(std::make_shared<PasswordScene>(page, screen));
  scenes.push_back(std::make_shared<MainScene>(page, screen));

  std::vector<Component> components;
  for (auto &scene : scenes) {
    components.push_back(scene->getComponent());
  }
  auto container = Container::Tab(components, page.get());

  static std::atomic<bool> running(true);
  static std::thread worker([scenes, page] {
    while (running) {
      if (state::changeState != state::ChangingAuthState::LOADING &&
          state::authState != state::AuthState::AUTHENTICATED) {
        (*page) = state::authState;
      } else if (state::changeState != state::ChangingAuthState::LOADING) {
        (*page) = 4;
      } else {
        (*page) = 0;
      }
			UniqueLogger::getInstance().debug("page: " + std::to_string(*page) + "\n\tstate::changeState: " + std::to_string(state::changeState) + "\n\tstate::authState: " + std::to_string(state::authState));
      scenes[*page]->ping();
      std::this_thread::sleep_for(500ms);
    }
  });
  static std::thread updater([] () {
    while (running) {
      TgFacade &tg_facade = TgFacade::getInstance();
      tg_facade.update_response();
      std::this_thread::sleep_for(1ms);
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
