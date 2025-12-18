#pragma once

#include <telegram.cpp>
#include <scenes/Scene.h>

#include <memory>
#include <string>

class PhoneScene : public Scene {
private:
  struct Components {
    Component input_field;
    Component continue_button;
    Component quit_button;
  };
  std::shared_ptr<Components> components;
  std::string phone;
  std::string error;

public:
  PhoneScene(std::shared_ptr<int> page, ScreenInteractive &screen,
             Logger *logger);
  Component getComponent() override;
  Element getElement() override;
};
