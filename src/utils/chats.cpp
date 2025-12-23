#include <utils/chats.h>

#include <algorithm>
#include <memory>

ChatManager::ChatManager() {
	this->logger = &UniqueLogger::getInstance();
}

void ChatManager::addOrUpdateChat(int64_t chat_id, TdChat chat) {
	chats_[chat_id] = std::move(chat);
}

void ChatManager::updateChatTitle(int64_t chat_id, const std::string& title) {
	auto it = chats_.find(chat_id);
	if (it == chats_.end()) {
		logger->debug("In ChatManager was interapted updateChatTitle, but chat_id in chats_ wasn't found");
		return;
	}
	it->second->title_ = title;
}

std::vector<Chat> ChatManager::getSortedChats(int32_t chat_list_id) {
	std::vector<Chat> result;
	for (const auto& [chat_id, chat] : chats_) {
		for (const td_api::object_ptr<td_api::chatPosition>& position : chat->positions_) {
			if (position->list_->get_id() == chat_list_id) {
				result.push_back(Chat{
					id: chat_id,
					order: position->order_,
					title: chat->title_
				});
				break;
			}
		}
	}
	std::sort(result.begin(), result.end(), [] (const Chat& a, const Chat& b) {
		return a.order < b.order || a.order == b.order && a.id < b.id;
	});
	return result;
}

