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
  components->chat_list =
      Menu(&visible_chat_titles, &selected_visible_chat, create_autoscrolled());
  components->folders = Menu(&folder_titles, &selected_folder);
}

void MainScene::updateChatList() {
  chat_titles.clear();
	logger->debug("MainScene::updateChatList 1");
	for (const Chat &chat : chat_manager->getSortedChats(td_api::chatListMain::ID)) {
		chat_titles.push_back(chat.title);
  }
	logger->debug("MainScene::updateChatList 2");
  if (chat_titles.empty()) {
    chat_titles.push_back("No chats available");
  }

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  visible_chat_titles.clear();

  for (int i = std::max(0, chatMenuStart);
       i < std::max(0, int(std::min(int(this->chat_titles.size()),
                                    chatMenuStart + w.ws_row - 9)));
       i++) {
    visible_chat_titles.push_back(this->chat_titles.at(i));
  }
	logger->debug("MainScene::updateChatList 3");
}

MenuOption MainScene::create_autoscrolled() {
	std::shared_ptr<MenuOption> option = std::make_shared<MenuOption>();
	(*option) = MenuOption::VerticalAnimated();
	option->on_change = [this, option] {
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		selected_chat = selected_visible_chat + this->chatMenuStart;
		int delta = std::max(0, std::max(std::min(this->chatMenuStart,
                                              this->selected_chat - 1),
                                     this->selected_chat - w.ws_row + 11)) -
		this->chatMenuStart;
		this->chatMenuStart += delta;
		if (delta > 0) {
			selected_visible_chat -= 1;
		}
		if (delta < 0) {
			selected_visible_chat += 1;
		}
	};
	return *option;
}

Component MainScene::getComponent() {
  return Container::Vertical({
      components->chat_list,
      components->quit_button,
      // components->chat,
  });
}

Element MainScene::getElement() {
  updateChatList();
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return hbox({vbox({
                   text("chats") | bold | center,
                   separator(),
                   components->chat_list->Render() | border |
                       size(HEIGHT, EQUAL, w.ws_row - 10) | vscroll_indicator |
                       flex,
                    separator(),
                   components->quit_button->Render() | center |
                       size(WIDTH, EQUAL, 200),
               }) | size(WIDTH, EQUAL, 30),
               vbox({

               }) | size(WIDTH, EQUAL, w.ws_col - 30)}) |
         size(HEIGHT, EQUAL, w.ws_row);
}
