#include <telegram/facade.h>

TgFacade::TgFacade(Logger* logger) {
	td::ClientManager::execute(
			td_api::make_object<td_api::setLogVerbosityLevel>(0));
	HandlerManager* handler_manager = new HandlerManager(logger);
	auto client_manager1 = std::make_unique<td::ClientManager>();
	sender = new Sender(logger, handler_manager, std::move(client_manager1));
	auto client_manager2 = std::make_unique<td::ClientManager>();
	processor = new Processor(logger, handler_manager, sender, std::move(client_manager2));
	changeState = ChangingState::ENTERING;
}

TgFacade& TgFacade::getInstance(Logger* logger) {
	static TgFacade instance(logger);
	return instance;
}


void TgFacade::set_code(std::string code, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationCode>(code),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::set_phone(std::string phone, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone, nullptr),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::set_password(std::string password, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationPassword>(password),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::update_response() {
	processor->update_response();
}

