#pragma once

#include <telegram.cpp>
#include <scenes/scene.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

namespace td_api = td::td_api;

class MainScene : public Scene {
private:
  struct Components {
    Component quit_button;
    Component chat_list;
    Component folders;
    Component chat;
  };
  std::shared_ptr<Components> components;
  std::vector<std::string> chat_titles;
  std::vector<std::string> visible_chat_titles;
  std::vector<std::string> folder_titles;
  int selected_chat = 0;
  int selected_visible_chat = 0;
  int selected_folder = 0;
  int chatMenuStart = 0;

   bool isArchived(const td_api::chat &chat);
   int64_t getMainOrder(const td_api::chat* chat);
   void updateChatList();
   MenuOption createAutoscrolled();

public:
  MainScene(std::shared_ptr<int> page, ScreenInteractive &screen,
            Logger *logger);
  Component getComponent() override;
  Element getElement() override;
};
