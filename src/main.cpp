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

	ChatManager* chat_manager = new ChatManager();
  TgFacade &tg_facade = TgFacade::getInstance();
	tg_facade.set_chat_manager(chat_manager);

  screen.Loop(getRenderer(screen, chat_manager));
  logger->system(Logger::SystemMessages::STOP);
}
