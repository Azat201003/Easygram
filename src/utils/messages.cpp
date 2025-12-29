#include <utils/messages.h>
#include <telegram/facade.h>
#include <functional>

class NewMessageUpdateHandler : public UpdateHandler {
	void update(td_api::Object& object) override {
		td_api::updateNewMessage& update_new_message = static_cast<td_api::updateNewMessage&>(object);	
		if (!update_new_message.message_)
			return;

		MessageManager::getInstance().new_message(std::move(update_new_message.message_));
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
			for (int32_t i = 0; i < messages->total_count_; ++i) {
				if (!chat_message_ids_.contains({messages->messages_[i]->chat_id_, messages->messages_[i]->id_})) {
					chat_message_ids_.insert({messages->messages_[i]->chat_id_, messages->messages_[i]->id_});
					messages_[chat_id].push_front(std::move(messages->messages_[i]));
				}
			}
			if (messages_[chat_id].size() && messages->total_count_) {
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
	
	if (!chat_message_ids_.contains({message->chat_id_, message->id_})) {
		chat_message_ids_.insert({message->chat_id_, message->id_});
		messages_[message->chat_id_].push_back(std::move(message));
	}
}

