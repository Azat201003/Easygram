#include <renderer.h>
#include <telegram/facade.h>
#include <state.h>

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
      if (State::changeState != State::ChangingAuthState::LOADING &&
          State::authState != State::AuthState::AUTHENTICATED) {
        (*page) = State::authState;
      } else if (State::changeState == State::ChangingAuthState::LOADING) {
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
      TgFacade &tg_facade = TgFacade::getInstance();
      tg_facade.update_response();
      std::this_thread::sleep_for(5ms);
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
