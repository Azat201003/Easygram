#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <logger/logger.h>

#include <fstream>
#include <memory>

#include <renderer.h>
#include <telegram/facade.h>
#include <utils/chats.h>
#include <scenes/manager.h>

using namespace ftxui;

class Factory {
public:
	ScreenInteractive& screen;
	Factory(ScreenInteractive& screen) : screen(screen) {}
	virtual void build() = 0;
	Logger* logger;
	Component renderer;
};

class DefaultFactory : public Factory {
public:
	DefaultFactory(ScreenInteractive& screen) : Factory(screen) {}
	void build() override {
		ChatsManager* chats_manager = new ChatsManager();

		TgFacade::getInstance().set_chats_manager(chats_manager);
		this->renderer = getRenderer(this->screen, new SceneManager(this->screen, chats_manager));

		this->logger = &UniqueLogger::getInstance();
		this->logger->setOutputFileAsPrint("log");
	}
};

int main() {
	ScreenInteractive screen = ScreenInteractive::Fullscreen();
	Factory* factory = new DefaultFactory(screen);
	factory->build();
	factory->logger->system(Logger::SystemMessages::START);
	screen.Loop(factory->renderer);
	factory->logger->system(Logger::SystemMessages::STOP);
}
