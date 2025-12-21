#include <renderer.h>
#include <telegram/facade.h>
#include <state.h>
#include <scenes/manager.h>
#include <ftxui/dom/elements.hpp>

Component getRenderer(ScreenInteractive &screen, SceneManager* scene_manager) {
  auto container = Container::Tab(
		scene_manager->get_components(),
		scene_manager->get_page().get()
	);

  static std::atomic<bool> running(true);
  auto& scenes = scene_manager->get_scenes();
  static std::thread worker([scene_manager] {
    while (running) {
      scene_manager->update();
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


	std::shared_ptr<int> page = scene_manager->get_page();
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
