#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <logger/logger.h>

#include <fstream>

#include <renderer.h>
#include <telegram/facade.h>

using namespace ftxui;

int main() {
  auto screen = ScreenInteractive::Fullscreen();

  Logger *logger = &UniqueLogger::getInstance();
  logger->setOutputFileAsPrint("log");
  logger->system(Logger::SystemMessages::START);

  TgFacade &tg_facade = TgFacade::getInstance();

  screen.Loop(getRenderer(screen, logger));
  logger->system(Logger::SystemMessages::STOP);
}
