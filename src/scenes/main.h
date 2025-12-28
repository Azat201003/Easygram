#pragma once

#include <telegram/facade.h>
#include <scenes/scene.h>
#include <utils/chats.h>
#include <utils/messages.h>

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
    Component input;
		Component resizable;
		Component selected_chat_title;
		Component submit_button;
  };
  std::shared_ptr<Components> components;
  std::vector<Chat> chats;
  std::vector<std::string> chat_titles;
  std::vector<std::string> folder_titles;
  int selected_chat = 0;
  int prev_selected_chat = 0;
	int selected_folder = 0;
	int left_width = 50;

	void updateChatList();
	void updateMessageList();
	MenuOption create_autoscrolled();

	ChatManager* chat_manager;
	MessageManager* message_manager;		
	std::string message_input;
public:
  MainScene(std::shared_ptr<int> page, ScreenInteractive &screen);
  Component getComponent() override;
  Element getElement() override;
};
