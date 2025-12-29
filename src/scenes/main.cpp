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
	this->message_manager = &MessageManager::getInstance();
	components = std::make_shared<Components>();
  
	components->quit_button = Button("Quit", screen.ExitLoopClosure());
	components->chat_list = Menu(&chat_titles, &selected_chat);
	components->folders = Menu(&folder_titles, &selected_folder);
	components->chat = Container::Vertical({});
	components->input = Input(&message_input, "Type message..");
	components->submit_button = Button("Send", [this] {
		if (chats.size() && selected_chat >= 0 && selected_chat < chats.size()) {
			auto send_message = td_api::make_object<td_api::sendMessage>();
			send_message->chat_id_ = chats[selected_chat].id;
			auto message_content = td_api::make_object<td_api::inputMessageText>();
			message_content->text_ = td_api::make_object<td_api::formattedText>();
			message_content->text_->text_ = message_input;
			send_message->input_message_content_ = std::move(message_content);

			TgFacade::getInstance().send_query(std::move(send_message), { });

			message_input = "";
		}
	});

	components->resizable = ResizableSplitLeft(
		Renderer(Container::Vertical({components->chat_list, components->quit_button}), [this] {
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			return vbox({
					text("Chats") | bold | center | size(HEIGHT, EQUAL, 3),
					separator(),
					components->chat_list->Render() | yframe |
						size(HEIGHT, EQUAL, w.ws_row - 7),
					separator(),
					components->quit_button->Render() | center | size(HEIGHT, EQUAL, 4),
			}) | size(HEIGHT, EQUAL, w.ws_row);
		}),
		Renderer(Container::Vertical({components->chat, Container::Horizontal({components->input, components->submit_button})}), [this] {
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			std::string chat_title = "Unselected";
			if (chats.size() && selected_chat >= 0 && selected_chat < chats.size())
				chat_title = chats[selected_chat].title;
			return vbox({
				text(chat_title) | bold | center | size(HEIGHT, EQUAL, 3),
				separator(),
				components->chat->Render() | yframe | size(HEIGHT, EQUAL, w.ws_row - 9),
				separator(),
				hbox({
					components->input->Render(),
					components->submit_button->Render() | size(WIDTH, EQUAL, 8) | center
				}) | size(HEIGHT, EQUAL, 5)
			});
		}),
		&left_width
	);
}

void MainScene::updateChatList() {
  if (chat_manager->updated) {
		chat_manager->updated = false;
		chat_titles.clear();
		chats = chat_manager->getSortedChats(td_api::chatListMain::ID);
		for (Chat chat : chats)
			chat_titles.push_back(chat.title);
	}
}

void MainScene::updateMessageList() {
	if (prev_selected_chat != selected_chat) {
		prev_selected_chat = selected_chat;
		message_manager->updated = false;
		message_manager->update_messages(chats[selected_chat].id);
	}
	if (message_manager->updated) {
		if (chats.size() && selected_chat >= 0 && selected_chat < chats.size()) {
			components->chat->DetachAllChildren();
			for (Message message : message_manager->get_messages(chats[selected_chat].id)) {
				MenuEntryOption option;
				option.transform = [message] (EntryState state) {
					Element e = paragraph(message.text) | size(WIDTH, LESS_THAN, 50);
					if (message.is_outgoing)
						e |= align_right;
					if (state.focused) {
						e = e | inverted;
					}
					if (state.active) {
						e = e | bold;
					}
					return e;
				};
				components->chat->Add(MenuEntry(message.text, option));
			}
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
	updateMessageList();
  return components->resizable->Render()	|
				 size(WIDTH, EQUAL, w.ws_col)			|
				 size(HEIGHT, EQUAL, w.ws_row)		;
}
