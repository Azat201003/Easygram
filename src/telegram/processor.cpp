#include <telegram/processor.h>
#include <state.h>
#include <config.h>
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

void Processor::process_response(td::ClientManager::Response response) {
	if (!response.object) {
		logger->named<Processor>("No response");
			return;
	}
	logger->named<Processor>(response.request_id + " " + 
													 to_string(response.object));
	if (response.request_id == 0) {
		return process_update(std::move(response.object));
	}
	auto it = sender->handlers_.find(response.request_id);
	if (it != sender->handlers_.end()) {
		it->second(std::move(response.object));
		sender->handlers_.erase(it);
	}
}

void Processor::process_update(Object update) {
	td_api::downcast_call(
			*update,
			overloaded(
					[this](
							td_api::updateAuthorizationState &update_authorization_state) {
						authorization_state_ =
								std::move(update_authorization_state.authorization_state_);
						on_authorization_state_update();
					},
					[this](td_api::updateNewChat &update_new_chat) {
						chats_[update_new_chat.chat_->id_] =
								std::move(update_new_chat.chat_);
						logger->debug("Update new chat handled");
					},
					 [this](td_api::updateChatPosition &update_chat_position) {
						 auto chat_it = chats_.find(update_chat_position.chat_id_);
						 if (chat_it != chats_.end()) {
							 auto chat = chat_it->second.get();
							 if (chat) {
								 bool found = false;
								 for (auto &pos : chat->positions_) {
									 if (pos && pos->list_->get_id() == update_chat_position.position_->list_->get_id()) {
										 pos = std::move(update_chat_position.position_);
										 found = true;
										 break;
									 }
								 }
								 if (!found) {
									 chat->positions_.push_back(std::move(update_chat_position.position_));
								 }
							 }
						 }
						 logger->debug("Update chat position handled");
					 },
					[this](td_api::updateChatTitle &update_chat_title) {
						chats_[update_chat_title.chat_id_]->title_ =
								update_chat_title.title_;
					},
					[this](td_api::updateUser &update_user) {
						auto user_id = update_user.user_->id_;
					},
					[this](td_api::updateNewMessage &update_new_message) {
						auto chat_id = update_new_message.message_->chat_id_;
						std::string sender_name;
						std::string text;
						if (update_new_message.message_->content_->get_id() ==
									td_api::messageText::ID) {
								text = static_cast<td_api::messageText &>(
													 *update_new_message.message_->content_)
													 .text_->text_;
						}
						logger->named<Processor>(
								"Receive message: [chat_id:" + to_string(chat_id) +
								"] [from:" + sender_name + "] [" + text + "]");
					},
					[](auto &update) {}));
}

void Processor::on_authorization_state_update() {
	authentication_query_id_++;
	state::changeState = state::ChangingAuthState::ENTERING;
	td_api::downcast_call(
			*authorization_state_,
			overloaded(
					[this](td_api::authorizationStateReady &) {
						state::authState = state::AuthState::AUTHENTICATED;
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
						state::authState = state::AuthState::CLOSED;
						logger->named<Processor>("Logging out");
					},
					[this](td_api::authorizationStateClosing &) {
						logger->named<Processor>("closed");
					},
					[this](td_api::authorizationStateClosed &) {
						state::authState = state::AuthState::CLOSED;
						state::needRestart = true;
						logger->named<Processor>("Terminated");
					},
					[this](td_api::authorizationStateWaitPhoneNumber &) {
						logger->named<Processor>("Entering phone number");
						state::authState = state::AuthState::PHONE_ENTER;
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
						state::authState = state::AuthState::CODE_ENTER;
					},
					[this](td_api::authorizationStateWaitRegistration &) {
					},
					[this](td_api::authorizationStateWaitPassword &) {
						logger->named<Processor>("Password entering");
						state::authState = state::AuthState::PASSWORD_ENTER;
					},
					[this](
							td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
						logger->named<Processor>(
								"This login link on another device confirmation" +
								state.link_);
					},
					[this](td_api::authorizationStateWaitTdlibParameters &) {
						state::authState = state::AuthState::TDLIB_PARAMS;
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
											 create_authentication_query_handler(&error));
					}));
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

std::function<void(Object)> Processor::create_authentication_query_handler(string *error) {
  return [this, id = authentication_query_id_, error](Object object) {
    logger->named<Processor>("authentication_query_handler\n");
    if (id == authentication_query_id_) {
      (*error) = check_authentication_error(std::move(object));
			state::changeState = state::ChangingAuthState::ERROR;
    }
	};
}
