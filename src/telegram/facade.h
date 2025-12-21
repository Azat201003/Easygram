#pragma once
#include <string>
#include <telegram/sender.h>
#include <telegram/processor.h>
#include <logger/logger.h>

class TgFacade {
private:
	enum ChangingState { ENTERING, LOADING, ERROR };
	ChangingState changeState;
	std::unique_ptr<td::ClientManager> client_manager_;
	Logger* logger;
public:
	TgFacade();
	static TgFacade& getInstance();
	Processor* processor;
	TgSender* sender;
	void set_code(std::string code, std::string* error);
	void set_phone(std::string phone, std::string* error);
	void set_password(std::string password, std::string* error);
	void update_response();
	void set_chats_manager(ChatsManager* chats_manager);
};

