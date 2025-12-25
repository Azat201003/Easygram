#pragma once
#include <string>
#include <telegram/sender.h>
#include <telegram/processor.h>
#include <logger/logger.h>
#include <utils/singleton.h>

// This class encapsulates, by delegating to other classes, methods for interacting with TDLib
class TgFacade : public Singleton<TgFacade> {
private:
	std::unique_ptr<td::ClientManager> client_manager_;
	Logger* logger;
	Processor* processor;
	TgSender* sender;
public:
	TgFacade();
	void set_code(std::string code, std::string* error);
	void set_phone(std::string phone, std::string* error);
	void set_password(std::string password, std::string* error);
	void send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler);
	void add_update_handler(int32_t object_id, UpdateHandler* update_handler);
	void update_response();
};

