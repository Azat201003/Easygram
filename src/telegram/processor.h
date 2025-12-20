#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <logger/logger.h>

#include <map>
#include <functional>

#include <telegram/sender.h>
#include <telegram/handlers.h>

namespace td_api = td::td_api;
using Object = td_api::object_ptr<td_api::Object>;

class Processor {
private:
	td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
	std::unique_ptr<td::ClientManager> client_manager_;
  void on_authorization_state_update();
	std::string check_authentication_error(Object object);
	std::uint64_t authentication_query_id_{0};
public:
	Logger* logger;
	std::function<void(Object)> create_authentication_query_handler(string *error);
	Sender* sender;
	HandlerManager* handler_manager;
	Processor(Logger*, HandlerManager*, Sender*, std::unique_ptr<td::ClientManager>);
	std::string error;
	std::map<std::int64_t, td_api::object_ptr<td_api::chat>> chats_;
	void process_response(td::ClientManager::Response response);
	void process_update(Object update);
	void update_response();
};
