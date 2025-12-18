#include <scenes/PhoneScene.h>

PhoneScene::PhoneScene(std::shared_ptr<int> page, ScreenInteractive &screen,
                       Logger *logger)
    : Scene(page, screen, logger) {
  components = std::make_shared<Components>();
  components->input_field = Input(&phone, "Enter your phone");
  components->quit_button = Button("Quit", screen.ExitLoopClosure());
  components->continue_button = Button("Continue", [this] {
    TdManager::getInstance().setPhoneNumber(phone, &error);
  });
}

Component PhoneScene::getComponent() {
  return Container::Vertical({components->input_field,
                              components->continue_button,
                              components->quit_button});
}

Element PhoneScene::getElement() {
  return vbox({
      hbox(text(" Login telegram with phone: ") | bold,
           components->input_field->Render()) |
          border | size(WIDTH, EQUAL, 50),
      filler(),
      hbox(components->continue_button->Render(), text(" "),
           components->quit_button->Render()),
      filler(),
      text(error) | color(Color::Red),
  });
}
