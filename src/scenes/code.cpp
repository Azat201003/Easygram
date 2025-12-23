#include <scenes/code.h>
#include <telegram/facade.h>

CodeScene::CodeScene(std::shared_ptr<int> page, ScreenInteractive &screen)
    : Scene(page, screen) {
  components = std::make_shared<Components>();
  components->input_field = Input(&code, "Enter your code");
  components->quit_button = Button("Quit", screen.ExitLoopClosure());
   components->continue_button = Button(
       "Continue", [this] { TgFacade::getInstance().set_code(code, &error); });
}

Component CodeScene::getComponent() {
  return Container::Vertical({components->input_field,
                              components->continue_button,
                              components->quit_button});
}

Element CodeScene::getElement() {
  return vbox({
      hbox(text("Enter your verification code: ") | bold,
           components->input_field->Render()) |
          border | size(WIDTH, EQUAL, 50),
      filler(),
      hbox(components->continue_button->Render(), text(" "),
           components->quit_button->Render()),
      filler(),
      text(error) | color(Color::Red),
  });
}
