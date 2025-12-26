#include <scenes/main.h>
#include <state.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

MainScene::MainScene(std::shared_ptr<int> page, ScreenInteractive &screen)
    : Scene(page, screen) {
  logger->debug("MainScene::MainScene");
	this->chat_manager = &ChatManager::getInstance();
	components = std::make_shared<Components>();
  components->quit_button = Button("Quit", screen.ExitLoopClosure());
  updateChatList();
  components->chat_list = Menu(&chat_titles, &selected_chat);
  components->folders = Menu(&folder_titles, &selected_folder);
  components->chat = Renderer([] { return text("Here will be messages") | borderEmpty; });
  
	components->resizable = ResizableSplitLeft(
		Renderer(Container::Vertical({components->chat_list, components->quit_button}), [this] {
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			return vbox({
					text("chats") | bold | center,
					separator(),
					components->chat_list->Render() | yframe | border |
						size(HEIGHT, EQUAL, w.ws_row - 8),
					separator(),
					components->quit_button->Render() | center,
			}) | size(HEIGHT, EQUAL, w.ws_row);
		}),
		Renderer([this] {
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			return vbox({

			});
		}),
		&left_width
	);
}

void MainScene::updateChatList() {
  if (chat_manager->updated) {
		chat_manager->updated = false;
		chat_titles.clear();
		logger->debug("MainScene::updateChatList 1");
		for (const Chat &chat : chat_manager->getSortedChats(td_api::chatListMain::ID)) {
			chat_titles.push_back(chat.title);
		}
		logger->debug("MainScene::updateChatList 2");
		if (chat_titles.empty()) {
			chat_titles.push_back("No chats available");
		}
	}
}

Component MainScene::getComponent() {
	return components->resizable;
}

Element MainScene::getElement() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  updateChatList();
  return components->resizable->Render()	|
				 size(WIDTH, EQUAL, w.ws_col)			|
				 size(HEIGHT, EQUAL, w.ws_row)		;
}
