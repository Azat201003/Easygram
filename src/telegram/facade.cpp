#include <telegram/facade.h>

TgFacade::TgFacade() {
	td::ClientManager::execute(
			td_api::make_object<td_api::setLogVerbosityLevel>(0));
	this->logger = &UniqueLogger::getInstance();
	HandlerManager* handler_manager = new HandlerManager();
	sender		= new TgSender(handler_manager);
	processor	= new Processor(handler_manager, sender);
	changeState = ChangingState::ENTERING;
	sender->send_query(td_api::make_object<td_api::getOption>("version"), {});
}

void TgFacade::set_chat_manager(ChatManager* chat_manager) {
	this->processor->set_chat_manager(chat_manager);
}

TgFacade& TgFacade::getInstance() {
	static TgFacade instance;
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

