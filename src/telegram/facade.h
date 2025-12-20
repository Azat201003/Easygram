#pragma once
#include <string>
#include <telegram/sender.h>
#include <telegram/processor.h>
#include <logger/logger.h>

class TgFacade {
private:
	enum ChangingState { ENTERING, LOADING, ERROR };
	ChangingState changeState;
public:
	TgFacade(Logger* logger);
	static TgFacade& getInstance(Logger* logger);
	Processor* processor;
	Sender* sender;
	Logger* logger;
	void set_code(std::string code, std::string* error);
	void set_phone(std::string phone, std::string* error);
	void set_password(std::string password, std::string* error);
	void update_response();
};

