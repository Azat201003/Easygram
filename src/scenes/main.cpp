#include <scenes/main.h>
#include <state.h>
#include <utils/chats.h>

#include <algorithm>
#include <cmath>
#include <string>



void MainScene::updateChatList() {
  chat_titles = chats_manager->getSortedChatTitles();

  // Clamp selected_chat to valid range
  if (!chat_titles.empty()) {
    selected_chat = std::min(selected_chat, (int)chat_titles.size() - 1);
    selected_chat = std::max(selected_chat, 0);
  } else {
    selected_chat = 0;
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
}

MenuOption MainScene::createAutoscrolled() {
  std::shared_ptr<MenuOption> option = std::make_shared<MenuOption>();
  (*option) = MenuOption::VerticalAnimated();
  option->on_change = [this, option] {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    selected_chat = selected_visible_chat + this->chatMenuStart;
    // Clamp selected_chat
    if (!chat_titles.empty()) {
      selected_chat = std::min(selected_chat, (int)chat_titles.size() - 1);
      selected_chat = std::max(selected_chat, 0);
    } else {
      selected_chat = 0;
    }
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

MainScene::MainScene(std::shared_ptr<int> page, ScreenInteractive &screen, ChatsManager* chats_manager_)
    : Scene(page, screen),
			chats_manager(chats_manager_) {
  components = std::make_shared<Components>();
  components->quit_button = Button("Quit", screen.ExitLoopClosure());
  updateChatList();
  components->chat_list =
      Menu(&visible_chat_titles, &selected_visible_chat, createAutoscrolled());
  components->folders = Menu(&folder_titles, &selected_folder);
}

Component MainScene::getComponent() {
  return Container::Vertical({
      components->chat_list,
      components->quit_button,
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
