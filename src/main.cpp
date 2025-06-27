#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <fstream>

#include "scenes.cpp"
#include "telegram.cpp"

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::Fullscreen();

  TdManager& tdManager = TdManager::getInstance();
  // cout << "hello world";
  std::ofstream outfile;
  outfile.open("abeme", std::ios_base::trunc);

  screen.Loop(getRenderer(screen));
}