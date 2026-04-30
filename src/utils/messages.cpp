#include <utils/messages.h>
#include <telegram/facade.h>
#include <functional>

class NewMessageUpdateHandler : public UpdateHandler {
	void update(Object object) override {
		auto update_new_message = td::move_tl_object_as<td_api::updateNewMessage>(object);	
		if (!update_new_message->message_)
			return;

		MessageManager::getInstance().new_message(std::move(update_new_message->message_));
		UniqueLogger::getInstance().debug("NewMessageUpdateHandler: new event");
	}
};

MessageManager::MessageManager() {
	TgFacade::getInstance().add_update_handler(td_api::updateNewMessage::ID, new NewMessageUpdateHandler());
}

void MessageManager::update_messages(int64_t chat_id) {
	TgFacade::getInstance().send_query(td_api::make_object<td_api::getChatHistory>(
			chat_id,
			first_message_id_[chat_id],
			0,
			20,
			false
		), [this, chat_id] (Object object) {
			td_api::object_ptr<td_api::messages> messages = td_api::move_object_as<td_api::messages>(object);
			if (!messages) return;

			std::lock_guard<std::mutex> lock(mtx_);
			for (auto &msg : messages->messages_) {
				if (!chat_message_ids_.count({msg->chat_id_, msg->id_})) {
					chat_message_ids_.insert({msg->chat_id_, msg->id_});
					messages_[chat_id].push_front(std::move(msg));
				}
			}
			if (!messages_[chat_id].empty()) {
				first_message_id_[chat_id] = (*messages_[chat_id].begin())->id_;
				UniqueLogger::getInstance().debug("MessageManager::update_messages new messages handled");
				updated = true;
			}
		}
	);
}

std::vector<Message> MessageManager::get_messages(int64_t chat_id) {
	std::vector<Message> r;
	for (const TdMessage& message: messages_[chat_id]) {
		if (message->content_->get_id() == td_api::messageText::ID) {
			r.push_back(Message{
				static_cast<const td_api::messageText&>(*message->content_).text_->text_,
				message->chat_id_,
				message->is_outgoing_	
			});
		}
	}
	return r;
}

void MessageManager::new_message(TdMessage message) {
	updated = true;
	
	std::lock_guard<std::mutex> lock(mtx_);
	if (!chat_message_ids_.count({message->chat_id_, message->id_})) {
		chat_message_ids_.insert({message->chat_id_, message->id_});
		messages_[message->chat_id_].push_back(std::move(message));
	}
}

