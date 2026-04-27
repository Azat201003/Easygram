#include <telegram/facade.h>
#include <state.h>
#include <memory>
#include <utils/config.h>

TgFacade::TgFacade() {
	td::ClientManager::execute(
			td_api::make_object<td_api::setLogVerbosityLevel>(0));
	this->logger = &UniqueLogger::getInstance();
	HandlerManager* handler_manager = new HandlerManager();
	sender		= new TgSender(handler_manager);
	processor	= new Processor(handler_manager, sender);
	UniqueLogger::getInstance().debug((Config::PROXY_ENABLED?"enabled ":"disabled ") + Config::PROXY_SERVER + ":" + to_string(Config::PROXY_PORT) + ", secret: " + Config::PROXY_SECRET);
	
	// Add proxy
	sender->send_query(td_api::make_object<td_api::addProxy>(
		td_api::make_object<td_api::proxy>(
			Config::PROXY_SERVER,
			Config::PROXY_PORT,
			td_api::make_object<td_api::proxyTypeMtproto>(
				Config::PROXY_SECRET
			)
		),
		Config::PROXY_ENABLED
	), [&] (Object object) {
		if (object->get_id() == td_api::addedProxy::ID) {
			UniqueLogger::getInstance().debug("Proxy added!");

			auto added_proxy = td::move_tl_object_as<td_api::addedProxy>(object);
			
			sender->send_query(
				td_api::make_object<td_api::pingProxy>(std::move(added_proxy->proxy_)),
				[](Object ping_obj) {
					if (ping_obj->get_id() == td_api::seconds::ID) {
						auto sec = td::move_tl_object_as<td_api::seconds>(ping_obj);
						UniqueLogger::getInstance().debug("Proxy works! Ping: " + to_string(sec->seconds_)  + "s");
					} else {
						std::cerr << "Proxy failed.\n";
						UniqueLogger::getInstance().debug("Proxy failed!");
					}
				}
			);
        }
	});

	// Loading telegram
	sender->send_query(td_api::make_object<td_api::getOption>("version"), {});
}

void TgFacade::add_update_handler(int32_t object_id, UpdateHandler* update_handler) {
	this->processor->add_update_handler(object_id, update_handler);
}

void TgFacade::send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler) {
	sender->send_query(std::move(f), handler);
}

void TgFacade::set_code(std::string code, std::string* error) {
	state::changeState = state::ChangingAuthState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationCode>(code),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::set_phone(std::string phone, std::string* error) {
	state::changeState = state::ChangingAuthState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone, nullptr),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::set_password(std::string password, std::string* error) {
	state::changeState = state::ChangingAuthState::LOADING;
	sender->send_query(
		td_api::make_object<td_api::checkAuthenticationPassword>(password),
		processor->create_authentication_query_handler(error)
	);
}

void TgFacade::update_response() {
	processor->update_response();
}

