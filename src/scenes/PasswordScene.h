#pragma once

#include "../telegram.cpp"
#include "Scene.h"

#include <memory>
#include <string>

class PasswordScene : public Scene {
private:
  struct Components {
    Component input_field;
    Component continue_button;
    Component quit_button;
  };
  std::shared_ptr<Components> components;
  std::string password;
  std::string error;

public:
  PasswordScene(std::shared_ptr<int> page, ScreenInteractive &screen,
                Logger *logger);
  Component getComponent() override;
  Element getElement() override;
};