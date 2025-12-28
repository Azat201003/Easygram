#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <logger/logger.h>

#include <vector>
#include <map>
#include <string>
#include <deque>
#include <set>

#include <utils/singleton.h>
#include <telegram/facade.h>


typedef td_api::object_ptr<td_api::message> TdMessage;

struct Message {
	std::string	text;
	int64_t chat_id;
	bool is_outgoing;
};

class MessageManager : public Singleton<MessageManager> {
public:
	void update_messages(int64_t chat_id);
	std::vector<Message> get_messages(int64_t chat_id);
	void new_message(TdMessage);
	bool updated = false;
	MessageManager();
private:
	std::set<int64_t> message_ids_;
	std::map<int64_t, int64_t> first_message_id_;
	std::map<int64_t, std::deque<TdMessage>> messages_; // by chat_id
};

