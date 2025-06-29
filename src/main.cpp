#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <logger/logger.h>

#include <fstream>

#include "scenes.cpp"
#include "telegram.cpp"

using namespace ftxui;

ofstream log;

void printLog(string text) {
  log << text;
}

int main() {
  auto screen = ScreenInteractive::Fullscreen();

  // cout << "hello world";
  log.open("log", std::ios_base::trunc);
  log.close();
  log.open("log", std::ios_base::app);
  if (!log.is_open()) {
    Logger().system(Logger::SystemMessages::STOP);
    return 0;
  }

  Logger* logger = new Logger();
  // logger->info("dsaf");
  logger->setPrintFunc(printLog);
  // logger->info("dsaf");
  logger->system(Logger::SystemMessages::START);


  TdManager& tdManager = TdManager::getInstance();
  tdManager.setLogger(logger);

  screen.Loop(getRenderer(screen, logger));
  logger->system(Logger::SystemMessages::STOP);
  log.close();
  return 0;
}
