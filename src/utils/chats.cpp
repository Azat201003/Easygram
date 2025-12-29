#include <utils/chats.h>

#include <algorithm>
#include <memory>

class NewChatUpdateHandler : public UpdateHandler {
public:
	void update(td_api::Object &object) override {
		td_api::updateNewChat& update_new_chat = static_cast<td_api::updateNewChat&>(object); 
		UniqueLogger::getInstance().debug("Update new chat handled");
		if (update_new_chat.chat_ == nullptr)
			return;

		td_api::chat* chat = update_new_chat.chat_.release();
		UniqueLogger::getInstance().debug("Chat: \"" + chat->title_ + "\", " + std::to_string(chat->id_));
		ChatManager::getInstance().addOrUpdateChat(chat->id_, td_api::object_ptr(chat));	
	}
};

class ChatPositionUpdateHandler : public UpdateHandler {
public:
	void update(td_api::Object &object) override {
		td_api::updateChatPosition& update_chat_position = static_cast<td_api::updateChatPosition&>(object); 

		UniqueLogger::getInstance().debug("Update chat position handled");
		ChatManager::getInstance().updateChatPosition(update_chat_position.chat_id_, std::move(update_chat_position.position_));
	}
};

class ChatLastMessageUpdateHandler : public UpdateHandler {
public:
	void update(td_api::Object &object) override {
		td_api::updateChatLastMessage& update_last_message = static_cast<td_api::updateChatLastMessage&>(object); 

		UniqueLogger::getInstance().debug("Update chat last message handled");
		ChatManager::getInstance().updateChatPositions(update_last_message.chat_id_, std::move(update_last_message.positions_));
	}
};

ChatManager::ChatManager() {
	this->logger = &UniqueLogger::getInstance();
	TgFacade::getInstance().add_update_handler(td_api::updateNewChat::ID, new NewChatUpdateHandler());
	TgFacade::getInstance().add_update_handler(td_api::updateChatPosition::ID, new ChatPositionUpdateHandler());
	TgFacade::getInstance().add_update_handler(td_api::updateChatLastMessage::ID, new ChatLastMessageUpdateHandler());
}

void ChatManager::addOrUpdateChat(int64_t chat_id, TdChat chat) {
	logger->debug("ChatManager::addOrUpdateChat");
	if (chat)
		chats_[chat_id] = std::move(chat);
	updated = true;
}

void ChatManager::updateChatPosition(int64_t chat_id, td_api::object_ptr<td_api::chatPosition> position) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatPosition, but chat_id in chats_ wasn't found");
		return;
	}
	bool found = false;
	td_api::chat* chat = it->second.get();
	for (auto &pos : chat->positions_) {
		if (pos && pos->list_->get_id() == position->list_->get_id()) {
			pos = std::move(position);
			found = true;
			break;
		}
	}
	if (!found) {
		chat->positions_.push_back(std::move(position));
	}
	updated = true;
	logger->debug("In ChatManager was interupted updateChatPosition successful, found: " + std::to_string(found));
}

void ChatManager::updateChatPositions(int64_t chat_id, std::vector<td_api::object_ptr<td_api::chatPosition>> positions) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatPositions, but chat_id in chats_ wasn't found");
		return;
	}
	it->second->positions_ = std::move(positions);
	
	updated = true;
	logger->debug("In ChatManager was interupted updateChatPositions successful");
}

void ChatManager::updateChatTitle(int64_t chat_id, const std::string& title) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatTitle, but chat_id in chats_ wasn't found");
		return;
	}
	if(std::to_address(it) == nullptr) {
		logger->debug("In ChatManager was interupted updateChatTitle, but chat is nullptr");
		return;
	}	
	it->second->title_ = title;
}

std::vector<Chat> ChatManager::getSortedChats(int32_t chat_list_id) {
	std::vector<Chat> result;
	for (const auto& [chat_id, chat] : chats_) {
		if (!chat)
			continue;
		logger->debug("ChatManager::getSortedChats checking" + chat->title_);
		for (const td_api::object_ptr<td_api::chatPosition>& position : chat->positions_) {
			if (position && position->list_ && position->list_->get_id() == chat_list_id) {
				result.push_back(Chat{
					id: chat_id,
					order: position->order_,
					title: (chat->title_ != ""?chat->title_:"Deleted")
				});
				break;
			}
		}
	}
	std::sort(result.begin(), result.end(), [] (const Chat& a, const Chat& b) {
		return a.order > b.order || a.order == b.order && a.id < b.id;
	});
	logger->debug("ChatManager::getSortedChats");
	return result;
}

