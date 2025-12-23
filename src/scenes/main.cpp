#include <scenes/main.h>
#include <state.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>

bool MainScene::isArchived(const td_api::chat &chat) {
  for (const auto &chatList : chat.chat_lists_) {
    if (chatList && chatList->get_id() == td_api::chatListArchive::ID) {
      return true;
    }
  }
  return false;
}

int64_t MainScene::getMainOrder(const td_api::chat* chat) {
  for (const auto& pos : chat->positions_) {
    if (pos && pos->list_->get_id() == td_api::chatListMain::ID) {
      return pos->order_;
    }
  }
  return 0;
}

void MainScene::updateChatList() {
  chat_titles.clear();
  std::vector<const td_api::chat*> chat_list;
   for (const auto &chat_pair : chats) {
    const auto &chat = chat_pair.second.get();
    if (chat && !isArchived(*chat)) {
      chat_list.push_back(chat);
    }
  }

    std::sort(chat_list.begin(), chat_list.end(), [this](const td_api::chat* a, const td_api::chat* b) {
      int64_t oa = getMainOrder(a);
      int64_t ob = getMainOrder(b);
      if (oa != 0 && ob != 0) {
        if (oa != ob) return oa > ob; // higher order first
      } else if (oa == 0 && ob == 0) {
        // fallback to date
        int64_t da = a->last_message_ ? a->last_message_->date_ : 0;
        int64_t db = b->last_message_ ? b->last_message_->date_ : 0;
        if (da != db) return da > db;
        return a->title_ < b->title_;
      } else {
        return oa > ob; // chats with position first
      }
      return a->id_ < b->id_; // fallback
    });
  std::unordered_set<std::string> added;
  for (const auto &chat : chat_list) {
    if (added.insert(chat->title_).second) {
      chat_titles.push_back(chat->title_);
    }
  }
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
}

MenuOption MainScene::createAutoscrolled() {
  std::shared_ptr<MenuOption> option = std::make_shared<MenuOption>();
  (*option) = MenuOption::VerticalAnimated();
  option->on_change = [this, option] {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // if ((chatMenuStart != 0 || selected_visible_chat != 0) && (chatMenuStart
    // + w.ws_row - 11 != chat_titles.size() || selected_visible_chat !=
    // this->selected_chat + w.ws_row - 11)) {
    selected_chat = selected_visible_chat + this->chatMenuStart;
     // }
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
    // vector<string> entries;
    // option->entries = entries;
    // option->entries = vector<std::copy(chat_titles.begin() + chatMenuStart,
    // chat_titles.begin()+chatMenuStart+w.ws_row,
    // std::back_inserter(chat_titles))>;
  };
  return *option;
}

MainScene::MainScene(std::shared_ptr<int> page, ScreenInteractive &screen)
    : Scene(page, screen) {
  components = std::make_shared<Components>();
  components->quit_button = Button("Quit", screen.ExitLoopClosure());
  updateChatList();
  components->chat_list =
      Menu(&visible_chat_titles, &selected_visible_chat, createAutoscrolled());
  components->folders = Menu(&folder_titles, &selected_folder);
  // components->chat = Menu();
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
