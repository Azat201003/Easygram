#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <logger/logger.h>

#include <fstream>

#include <renderer.h>
#include <telegram/facade.h>
#include <utils/config.h>

using namespace ftxui;

int main(int argc, const char *argv[]) {
  auto screen = ScreenInteractive::Fullscreen();

  Logger *logger = &UniqueLogger::getInstance();
  logger->setOutputFileAsPrint("log");
  logger->system(Logger::SystemMessages::START);

  int config_status = Config::init(argc, argv);
  if (config_status) return config_status;
  logger->system("Config loaded");

  TgFacade &tg_facade = TgFacade::getInstance();

  screen.Loop(getRenderer(screen));
  logger->system(Logger::SystemMessages::STOP);
}
