#pragma once

#include <scenes/Scene.h>

class LoadingScene : public Scene {
private:
  int n = 0;
  const char chars[26] = "A!$XLW)!*#:D&!)!)XPZ:pqix";

public:
  LoadingScene(std::shared_ptr<int> page, ScreenInteractive &screen,
               Logger *logger);
  Component getComponent() override;
  Element getElement() override;
};
