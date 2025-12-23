#pragma once

#include <scenes/scene.h>

#include <memory>
#include <string>

class CodeScene : public Scene {
private:
  struct Components {
    Component input_field;
    Component continue_button;
    Component quit_button;
  };
  std::shared_ptr<Components> components;
  std::string code;
  std::string error;

public:
  CodeScene(std::shared_ptr<int> page, ScreenInteractive &screen,
            Logger *logger);
  Component getComponent() override;
  Element getElement() override;
};
