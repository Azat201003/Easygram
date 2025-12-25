#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <logger/logger.h>

#include <telegram/sender.h>
#include <telegram/handlers.h>

#include <map>
#include <vector>
#include <functional>

namespace td_api = td::td_api;
using Object = td_api::object_ptr<td_api::Object>;

class UpdateHandler {
public:
	virtual void update(td_api::Object&) = 0;
};

class Processor {
private:
	td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
	td::ClientManager* client_manager_;
  void on_authorization_state_update();
	std::string check_authentication_error(Object object);
	std::uint64_t authentication_query_id_{0};
	Logger* logger;
	TgSender* sender;
	HandlerManager* handler_manager;
	std::map<int32_t, std::vector<UpdateHandler*>> update_handlers; // by Function::ID
public:
	std::function<void(Object)> create_authentication_query_handler(string *error);
	Processor(HandlerManager*, TgSender*);
	std::string error;

	void add_update_handler(int32_t object_id, UpdateHandler* update_handler);

	void process_response(td::ClientManager::Response response);
	void process_update(Object update);
	void update_response();
};
