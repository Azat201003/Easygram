#include <utils/chats.h>

#include <algorithm>
#include <memory>

class NewChatUpdateHandler : public UpdateHandler {
public:
	void update(Object object) override {
		auto update_new_chat = td::move_tl_object_as<td_api::updateNewChat>(object); 
		UniqueLogger::getInstance().debug("Update new chat handled");
		if (update_new_chat->chat_ == nullptr)
			return;

		auto chat = move(update_new_chat->chat_);
		UniqueLogger::getInstance().debug("Chat: \"" + chat->title_ + "\", " + std::to_string(chat->id_));
		td_api::int53 chat_id = chat->id_;
		ChatManager::getInstance().addOrUpdateChat(chat_id, std::move(chat));	
	}
};

class ChatPositionUpdateHandler : public UpdateHandler {
public:
	void update(Object object) override {
		auto update_chat_position = td::move_tl_object_as<td_api::updateChatPosition>(object);

		UniqueLogger::getInstance().debug("Update chat position handled");
		ChatManager::getInstance().updateChatPosition(update_chat_position->chat_id_, std::move(update_chat_position->position_));
	}
};

class ChatLastMessageUpdateHandler : public UpdateHandler {
public:
	void update(Object object) override {
		auto update_last_message = td::move_tl_object_as<td_api::updateChatLastMessage>(object);

		UniqueLogger::getInstance().debug("Update chat last message handled");
		ChatManager::getInstance().updateChatPositions(update_last_message->chat_id_, std::move(update_last_message->positions_));
	}
};

td_api::object_ptr<td_api::chatPosition> extractMainListPosition(td::td_api::array<td::td_api::object_ptr<td::td_api::chatPosition>> positions) {
	for (td_api::object_ptr<td_api::chatPosition>& position : positions) {
		if (position && position->list_ && position->list_->get_id() == td_api::chatListMain::ID) {
			return std::move(position);
		}
	}
	return nullptr;
}

void Chat::parseTdChat(TdChat chat) {
	id = chat->id_;
	title = chat->title_;
	order = 0;
	auto position = extractMainListPosition(std::move(chat->positions_));
	if (position)
		order = position->order_;
}

ChatManager::ChatManager() {
	this->logger = &UniqueLogger::getInstance();
	TgFacade::getInstance().add_update_handler(td_api::updateNewChat::ID, new NewChatUpdateHandler());
	TgFacade::getInstance().add_update_handler(td_api::updateChatPosition::ID, new ChatPositionUpdateHandler());
	TgFacade::getInstance().add_update_handler(td_api::updateChatLastMessage::ID, new ChatLastMessageUpdateHandler());
}

void ChatManager::addOrUpdateChat(int64_t chat_id, TdChat chat) {
	logger->debug("ChatManager::addOrUpdateChat, chat_id: " + to_string(chat_id));
	chats_[chat_id].parseTdChat(std::move(chat));
	updated = true;
}

void ChatManager::updateChatPosition(int64_t chat_id, td_api::object_ptr<td_api::chatPosition> position) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatPosition, but chat_id in chats_ wasn't found");
		return;
	}

	if (position->list_->get_id() != td_api::chatListMain::ID) {
		logger->debug("position was altered in not main chat list");
		return;
	}

	it->second.order = position->order_;

	updated = true;
	logger->debug("In ChatManager was interupted updateChatPosition successful, found");
}

void ChatManager::updateChatPositions(int64_t chat_id, std::vector<td_api::object_ptr<td_api::chatPosition>> positions) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatPositions, but chat_id in chats_ wasn't found");
		return;
	}

	auto position = extractMainListPosition(std::move(positions));
	if (position.get() != nullptr)
		it->second.order = position->order_;
	
	updated = true;
	logger->debug("In ChatManager was interupted updateChatPositions successful");
}

void ChatManager::updateChatTitle(int64_t chat_id, const std::string& title) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatTitle, but chat_id in chats_ wasn't found");
		return;
	}
	if(std::addressof(it) == nullptr) {
		logger->debug("In ChatManager was interupted updateChatTitle, but chat is nullptr");
		return;
	}	
	it->second.title = title;
}

std::vector<Chat> ChatManager::getSortedChats(int32_t chat_list_id) {
	std::vector<Chat> result;
	for (auto [chat_id, chat] : chats_) {
		logger->debug("ChatManager::getSortedChats checking " + chat.title);
		result.push_back(Chat{
			id: chat.id,
			order: chat.order,
			title: (chat.title != ""?chat.title:"Deleted")
		});
	}
	std::sort(result.begin(), result.end(), [] (const Chat& a, const Chat& b) {
		return a.order > b.order || (a.order == b.order && a.id < b.id);
	});

	logger->debug("ChatManager::getSortedChats, result len: " + to_string(result.size()) + ", chats_ len: " + to_string(chats_.size()));
	return result;
}

