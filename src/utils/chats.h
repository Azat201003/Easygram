#pragma once

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <logger/logger.h>

#include <vector>
#include <map>
#include <string>

namespace td_api = td::td_api;
typedef td_api::object_ptr<td_api::chat> TdChat;

struct Chat {
	int64_t id;
	int64_t order;
	std::string title;
};

class ChatManager {
private:
	std::unordered_map<int64_t, TdChat> chats_;
	Logger* logger;
public:
	ChatManager();
	void addOrUpdateChat(int64_t chat_id, TdChat chat);
	void updateChatTitle(int64_t chat_id, const std::string& title);
	std::vector<Chat> getSortedChats(int32_t chat_list_id);
};

