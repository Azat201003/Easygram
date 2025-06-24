#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp> 
#include "utils/scenes.hpp"

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
  
    std::string phone;
    Component input_field = Input(&phone, "Input your phone: ");
    
    Component continue_button = Button("Continue", [&] {});

    Component quit_button = Button("Quit", screen.ExitLoopClosure());

    auto container = Container::Vertical({
        input_field,
        continue_button,
        quit_button
    });
    const Component GetPhonePage = Renderer(container, [&] { return vbox({
      hbox(text(" Поле ввода: ") | bold, input_field->Render()) | border | size(WIDTH, EQUAL, 50),
      filler(),
      hbox(continue_button->Render(), text(" "), quit_button->Render()),
      filler(),
    }) | border | size(HEIGHT, LESS_THAN, 10) | center;});

    

    screen.Loop(GetPhonePage);
}