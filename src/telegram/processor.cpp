#include <telegram/processor.h>
#include <state.h>
#include <config.h>
#include <utils/chats.h>
#include <thread>

namespace detail {
template <class... Fs> struct overload;

template <class F> struct overload<F> : public F {
  explicit overload(F f) : F(f) {}
};
template <class F, class... Fs>
struct overload<F, Fs...> : public overload<F>, public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
}

template <class... F> auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

Processor::Processor(HandlerManager* handler_manager_, TgSender* sender_) {
	this->logger = &UniqueLogger::getInstance();
	this->handler_manager = handler_manager_;
	this->sender = sender_;
	this->chats_manager = nullptr;
	this->client_manager_ = td::ClientManager::get_manager_singleton();
}

void Processor::set_chats_manager(ChatsManager* chats_manager) {
	this->chats_manager = chats_manager;
}

void Processor::process_response(td::ClientManager::Response response) {
	if (!response.object) {
		logger->named<Processor>("No response");
		return;
	}
	if (response.request_id) {
		handler_manager->process(response.request_id, std::move(response.object));
	} else {
		// process_update(std::move(response.object));
	}
	logger->named<Processor>(response.request_id + " " + 
													 to_string(response.object));
	if (response.request_id == 0) {
		return process_update(std::move(response.object));
	}
	handler_manager->process(response.request_id, std::move(response.object));
}

void Processor::process_update(Object update) {
	if (!update) return;
	logger->debug("Processing update");
	td_api::downcast_call(
		*update,
		overloaded(
			[this](td_api::updateAuthorizationState &update_authorization_state) {
				if (update_authorization_state.authorization_state_) {
					authorization_state_ =
						std::move(update_authorization_state.authorization_state_);
					on_authorization_state_update();
				}
			},
			[this](td_api::updateNewChat &update_new_chat) {
				if (update_new_chat.chat_ && chats_manager) {
					chats_manager->addOrUpdateChat(update_new_chat.chat_->id_,
						std::move(update_new_chat.chat_));
				}
				logger->debug("Update new chat handled");
			},
			[this](td_api::updateChatPosition &update_chat_position) {
				if (update_chat_position.position_ && chats_manager) {
					chats_manager->updateChatPosition(update_chat_position.chat_id_, std::move(update_chat_position.position_));
				}
				logger->debug("Update chat position handled");
			},
			[this](td_api::updateChatTitle &update_chat_title) {
				if (chats_manager) {
					chats_manager->updateChatTitle(update_chat_title.chat_id_,
							update_chat_title.title_);
				}
			},
			[this](td_api::updateUser &update_user) {
				if (update_user.user_) {
					auto user_id = update_user.user_->id_;
				}
			},
			[this](td_api::updateNewMessage &update_new_message) {
				if (update_new_message.message_ && update_new_message.message_->content_) {
					auto chat_id = update_new_message.message_->chat_id_;
					std::string sender_name;
					std::string text;
					if (update_new_message.message_->content_->get_id() ==
								td_api::messageText::ID) {
							auto& msg_text = static_cast<td_api::messageText &>(
												 *update_new_message.message_->content_);
							if (msg_text.text_) {
								text = msg_text.text_->text_;
							}
					}
					logger->named<Processor>(
							"Receive message: [chat_id:" + to_string(chat_id) +
							"] [from:" + sender_name + "] [" + text + "]");
				}
			},
			[](auto &update) {}
		)
	);
}

void Processor::on_authorization_state_update() {
	authentication_query_id_++;
	State::changeState = State::ChangingAuthState::ENTERING;
	td_api::downcast_call(
		*authorization_state_,
		overloaded(
			[this](td_api::authorizationStateReady &) {
				State::authState = State::AuthState::AUTHENTICATED;
				logger->named<Processor>("Authorization is completed");
					static std::thread loadChats([this] () {
						sender->send_query(td_api::make_object<td_api::loadChats>(
							td_api::make_object<td_api::chatListMain>(), 100), 
							[this] (Object object) {
								if (object->get_id() == td_api::error::ID) {
									this->logger->error("loading chats error");
									return;
								}
							}
						);
					});
			},
			[this](td_api::authorizationStateLoggingOut &) {
				State::authState = State::AuthState::CLOSED;
				logger->named<Processor>("Logging out");
			},
			[this](td_api::authorizationStateClosing &) {
				logger->named<Processor>("closed");
			},
			[this](td_api::authorizationStateClosed &) {
				State::authState = State::AuthState::CLOSED;
				State::needRestart = true;
				logger->named<Processor>("Terminated");
			},
			[this](td_api::authorizationStateWaitPhoneNumber &) {
				logger->named<Processor>("Entering phone number");
				State::authState = State::AuthState::PHONE_ENTER;
			},
			[this](td_api::authorizationStateWaitPremiumPurchase &) {
				logger->named<Processor>(
						"Telegram Premium subscription is required");
			},
			[this](td_api::authorizationStateWaitEmailAddress &) {
				logger->named<Processor>("Email address entering");
			},
			[this](td_api::authorizationStateWaitEmailCode &) {
				logger->named<Processor>("Email auth code entering");
			},
			[this](td_api::authorizationStateWaitCode &) {
				logger->named<Processor>("App auth code entering");
				State::authState = State::AuthState::CODE_ENTER;
			},
			[this](td_api::authorizationStateWaitRegistration &) {
			},
			[this](td_api::authorizationStateWaitPassword &) {
				logger->named<Processor>("Password entering");
				State::authState = State::AuthState::PASSWORD_ENTER;
			},
			[this](td_api::authorizationStateWaitOtherDeviceConfirmation &State) {
				logger->named<Processor>(
						"This login link on another device confirmation" +
						State.link_);
			},
			[this](td_api::authorizationStateWaitTdlibParameters &) {
				State::authState = State::AuthState::TDLIB_PARAMS;
				logger->named<Processor>(
						"AuthorizationStateWaitTdlibParameters entering");
				auto request = td_api::make_object<td_api::setTdlibParameters>();
				request->database_directory_ = "tdlib";
				request->use_message_database_ = true;
				request->use_secret_chats_ = true;
				request->api_id_ = config::API_ID;
				request->api_hash_ = config::API_HASH;
				request->system_language_code_ = "en";
				request->device_model_ = "Desktop";
				request->application_version_ = "1.0";
				string error;
				sender->send_query(std::move(request),
						create_authentication_query_handler(&error, State::AuthState::PHONE_ENTER));
			}
		)
	);
}

std::string Processor::check_authentication_error(Object object) {
	logger->named<Processor>("check_authentication_error");
	if (object->get_id() == td_api::error::ID) {
		error = to_string(td::move_tl_object_as<td_api::error>(object));
		on_authorization_state_update();
		return error;
	}
	return "";
}

std::function<void(Object)> Processor::create_authentication_query_handler(string *error, int next_state) {
  return [this, id = authentication_query_id_, error, next_state](Object object) {
    logger->named<Processor>("authentication_query_handler\n");
    if (id == authentication_query_id_) {
      (*error) = check_authentication_error(std::move(object));
			State::changeState = State::ChangingAuthState::ERROR;
			if ((*error).empty()) {
				State::authState = (State::AuthState)next_state;
			}
    }
  };
}

void Processor::update_response() {
	td::ClientManager::Response response;
  do {
	  response = client_manager_->receive(1);
  	process_response(std::move(response));
	} while (response.object);
}

