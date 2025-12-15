#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <logger/logger.h>

#include <fstream>

#include "scenes/Renderer.h"
#include "telegram.cpp"

using namespace ftxui;

ofstream log_file;

void printLog(string text) { log_file << text; }

int main() {
  auto screen = ScreenInteractive::Fullscreen();

  // cout << "hello world";
  log_file.open("log", std::ios_base::trunc);
  log_file.close();
  log_file.open("log", std::ios_base::app);
  if (!log_file.is_open()) {
    Logger().system(Logger::SystemMessages::STOP);
    return 0;
  }

  Logger *logger = new Logger();
  // logger->info("dsaf");
  logger->setPrintFunc(printLog);
  // logger->info("dsaf");
  logger->system(Logger::SystemMessages::START);

  TdManager &tdManager = TdManager::getInstance();
  tdManager.setLogger(logger);

  screen.Loop(getRenderer(screen, logger));
  logger->system(Logger::SystemMessages::STOP);
  log_file.close();
  return 0;
}
