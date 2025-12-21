#include "chats.h"
#include <unordered_set>
#include <mutex>
#include <logger/logger.h>
#include <string>

void ChatsManager::addOrUpdateChat(int64_t chat_id, td_api::object_ptr<td_api::chat> chat) {
	std::lock_guard<std::mutex> lock(mutex_);
	UniqueLogger::getInstance().debug(std::to_string(chat_id));
	auto it = chats_.find(chat_id);
	if (it != chats_.end()) {
		auto old_chat = it->second.get();
		if (old_chat) {
			sorted_chats_.erase(old_chat);
		}
	}
	auto& chat_ptr = chats_[chat_id] = std::move(chat);
	auto new_chat = chat_ptr.get();
	if (new_chat) {
		sorted_chats_.insert(new_chat);
	}
}

void ChatsManager::updateChatPosition(int64_t chat_id, td_api::object_ptr<td_api::chatPosition> position) {
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = chats_.find(chat_id);
	if (it != chats_.end()) {
		auto chat = it->second.get();
		if (chat) {
			sorted_chats_.erase(chat);
			bool found = false;
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
			sorted_chats_.insert(chat);
		}
	}
}

void ChatsManager::updateChatTitle(int64_t chat_id, const std::string& title) {
	std::lock_guard<std::mutex> lock(mutex_);
	auto it = chats_.find(chat_id);
	if (it != chats_.end()) {
		auto chat = it->second.get();
		if (chat) {
			sorted_chats_.erase(chat);
			it->second->title_ = title;
			sorted_chats_.insert(chat);
		}
	}
}

std::vector<std::string> ChatsManager::getSortedChatTitles() {
	std::lock_guard<std::mutex> lock(mutex_);
	std::vector<std::string> titles;
	std::unordered_set<std::string> added;
	for (const auto& chat : sorted_chats_) {
			if (chat && !isArchived(*chat)) {
					if (added.insert(chat->title_).second) {
							titles.push_back(chat->title_);
					}
			}
	}
	if (titles.empty()) {
			titles.push_back("No chats available");
	}
	return titles;
}

bool ChatsManager::isArchived(const td_api::chat& chat) const {
	for (const auto &chatList : chat.chat_lists_) {
			if (chatList && chatList->get_id() == td_api::chatListArchive::ID) {
					return true;
			}
	}
	return false;
}
