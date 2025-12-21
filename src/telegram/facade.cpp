#include <telegram/facade.h>

TgFacade::TgFacade() {
	td::ClientManager::execute(
			td_api::make_object<td_api::setLogVerbosityLevel>(0));
	
	HandlerManager* handler_manager = new HandlerManager();
	
	this->sender		= new TgSender(handler_manager);
	this->processor	= new Processor(handler_manager, sender);
	this->sender->send_query(td_api::make_object<td_api::getOption>("version"), {});
	

	changeState = ChangingState::ENTERING;

	this->logger = &UniqueLogger::getInstance();
}

TgFacade& TgFacade::getInstance() {
	static TgFacade instance;
	return instance;
}

void TgFacade::set_code(std::string code, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationCode>(code),
		processor->create_authentication_query_handler(error, State::AuthState::AUTHENTICATED)
	);
}

void TgFacade::set_phone(std::string phone, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone, nullptr),
		processor->create_authentication_query_handler(error, State::AuthState::CODE_ENTER)
	);
}

void TgFacade::set_password(std::string password, std::string* error) {
	changeState = ChangingState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationPassword>(password),
		processor->create_authentication_query_handler(error, State::AuthState::AUTHENTICATED)
	);
}

void TgFacade::update_response() {
	processor->update_response();
}

void TgFacade::set_chats_manager(ChatsManager* chats_manager) {
	processor->set_chats_manager(chats_manager);
}

