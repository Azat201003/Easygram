#pragma once

#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <mutex>

namespace td_api = td::td_api;

struct ChatComparator {
    bool operator()(const td_api::chat* a, const td_api::chat* b) const {
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
    }
private:
    int64_t getMainOrder(const td_api::chat* chat) const {
        for (const auto& pos : chat->positions_) {
            if (pos && pos->list_->get_id() == td_api::chatListMain::ID) {
                return pos->order_;
            }
        }
        return 0;
    }
};

class ChatsManager {
public:
    void addOrUpdateChat(int64_t chat_id, td_api::object_ptr<td_api::chat> chat);
    void updateChatPosition(int64_t chat_id, td_api::object_ptr<td_api::chatPosition> position);
    void updateChatTitle(int64_t chat_id, const std::string& title);
    std::vector<std::string> getSortedChatTitles();

private:
    std::unordered_map<int64_t, td_api::object_ptr<td_api::chat>> chats_;
    std::set<const td_api::chat*, ChatComparator> sorted_chats_;
    std::mutex mutex_;
    bool isArchived(const td_api::chat& chat) const;
};
