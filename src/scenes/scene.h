#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <logger/logger.h>

#include <memory>

using namespace ftxui;

class Scene {
protected:
  std::shared_ptr<int> page;
  Logger *logger;
  ScreenInteractive &screen;

public:
  Scene(std::shared_ptr<int> page, ScreenInteractive &screen, Logger *logger)
      : page(page), screen(screen), logger(logger) {}
  virtual ~Scene() = default;
  virtual Component getComponent() = 0;
  virtual Element getElement() = 0;
  void ping() {
    screen.PostEvent(Event::Special("Ping"));
  }
};
