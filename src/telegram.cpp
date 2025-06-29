#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <logger/logger.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

const int API_ID = 15307846;
const string API_HASH = "4132481003929021d07da880c84f2a33";

namespace detail {
    template <class... Fs>
    struct overload;

    template <class F>
    struct overload<F> : public F {
        explicit overload(F f) : F(f) {
        }
    };
    template <class F, class... Fs>
    struct overload<F, Fs...>
        : public overload<F>
        , public overload<Fs...> {
        overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {
        }
        using overload<F>::operator();
        using overload<Fs...>::operator();
    };
}

template <class... F>
auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

namespace td_api = td::td_api;

class TdManager {
public:
    enum AuthState {
        TDLIB_PARAMS,
        PHONE_ENTER,
        CODE_ENTER,
        PASSWORD_ENTER,
        AUTHENTICATED,
    };

    enum ChangingState {
        ENTERING,
        LOADING,
        ERROR,
    };

    AuthState authState = AuthState::TDLIB_PARAMS;
    ChangingState changeState = ChangingState::ENTERING;
    std::string error;
    std::map<std::int64_t, td_api::object_ptr<td_api::chat>> chats_;
private:
    Logger* logger = new Logger();
    auto create_authentication_query_handler(string* error) {
        return [this, id = authentication_query_id_, error](Object object) {
            logger->named<TdManager>("uthentication_query_handler\n");
            if (id == authentication_query_id_) {
                (*error) = check_authentication_error(std::move(object));
                changeState = ChangingState::ERROR;
            }
        };
        // chats_[0].get()->id_
    }
public:
    void setLogger(Logger* logger) {
        this->logger = logger;
    }

    static TdManager& getInstance() {
        static TdManager instance;
        return instance;
    }
    void update_response() {
        process_response(client_manager_->receive(15));
    }

    void setPhoneNumber(string phone, string* error) {
        changeState = ChangingState::LOADING;
        send_query(
            td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone, nullptr),
            create_authentication_query_handler(error));
    }

    void setCode(string code, string* error) {
        changeState = ChangingState::LOADING;
        send_query(td_api::make_object<td_api::checkAuthenticationCode>(code),
                create_authentication_query_handler(error));
    }

    void setPassword(string password, string* error) {
        changeState = ChangingState::LOADING;
        send_query(td_api::make_object<td_api::checkAuthenticationPassword>(password),
                create_authentication_query_handler(error));
    }
private:
    using Object = td_api::object_ptr<td_api::Object>; 
    TdManager() {
        td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
        client_manager_ = std::make_unique<td::ClientManager>();
        client_id_ = client_manager_->create_client_id();
        send_query(td_api::make_object<td_api::getOption>("version"), {});
    }

    void send_query(td_api::object_ptr<td_api::Function> f, std::function<void(Object)> handler) {
        auto query_id = next_query_id();
        if (handler) {
        handlers_.emplace(query_id, std::move(handler));
        }
        client_manager_->send(client_id_, query_id, std::move(f));
    }

    std::uint64_t next_query_id() {
        return ++current_query_id_;
    }

    void on_authorization_state_update() {
        authentication_query_id_++;
        changeState = ChangingState::ENTERING;
        td_api::downcast_call(*authorization_state_,
                            overloaded(
                                [this](td_api::authorizationStateReady &) {
                                    authState = AuthState::AUTHENTICATED;
                                    idAuthed = true;
                                    logger->named<TdManager>("Authorization is completed");
                                    send_query(td_api::make_object<td_api::getChats>(td_api::make_object<td_api::chatListMain>(), 100), [this] (Object object) {
                                        if (object->get_id() == td_api::error::ID) {
                                            return;
                                        }
                                        for (auto chat_id : td::move_tl_object_as<td_api::chats>(object)->chat_ids_) {
                                            send_query(td_api::make_object<td_api::getChat>(chat_id), [this, chat_id] (Object object) {
                                                this->chats_[chat_id] = td::move_tl_object_as<td_api::chat>(object);
                                            });
                                        }
                                    });
                                },
                                [this](td_api::authorizationStateLoggingOut &) {
                                    idAuthed = false;
                                    logger->named<TdManager>("Logging out");
                                },
                                [this](td_api::authorizationStateClosing &) { logger->named<TdManager>("closed"); },
                                [this](td_api::authorizationStateClosed &) {
                                    idAuthed = false;
                                    needRestart = true;
                                    logger->named<TdManager>("Terminated");
                                },
                                [this](td_api::authorizationStateWaitPhoneNumber &) {
                                    logger->named<TdManager>("Entering phone number");
                                    authState = AuthState::PHONE_ENTER;
                                },
                                [this](td_api::authorizationStateWaitPremiumPurchase &) {
                                    logger->named<TdManager>("Telegram Premium subscription is required");
                                },
                                [this](td_api::authorizationStateWaitEmailAddress &) {
                                    logger->named<TdManager>("Email address entering");
                                    // std::string email_address;
                                    // std::cin >> email_address;
                                    // send_query(td_api::make_object<td_api::setAuthenticationEmailAddress>(email_address),
                                    //         create_authentication_query_handler());
                                },
                                [this](td_api::authorizationStateWaitEmailCode &) {
                                    logger->named<TdManager>("Email auth code entering");
                                    // std::string code;
                                    // std::cin >> code;
                                    // send_query(td_api::make_object<td_api::checkAuthenticationEmailCode>(
                                    //             td_api::make_object<td_api::emailAddressAuthenticationCode>(code)),
                                    //         create_authentication_query_handler());
                                },
                                [this](td_api::authorizationStateWaitCode &) {
                                    logger->named<TdManager>("App auth code entering");
                                    authState = AuthState::CODE_ENTER;
                                },
                                [this](td_api::authorizationStateWaitRegistration &) {
                                    // std::string first_name;
                                    // std::string last_name;
                                    // cout << "Enter your first name: " << std::flush;
                                    // outfile.close();
                                    // std::cin >> first_name;
                                    // cout << "Enter your last name: " << std::flush;
                                    // outfile.close();
                                    // std::cin >> last_name;
                                    // send_query(td_api::make_object<td_api::registerUser>(first_name, last_name, false),
                                    //         create_authentication_query_handler());
                                },
                                [this](td_api::authorizationStateWaitPassword &) {
                                    logger->named<TdManager>("Password entering");
                                    authState = AuthState::PASSWORD_ENTER;
                                },
                                [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
                                    logger->named<TdManager>("This login link on another device confirmation" + state.link_);
                                },
                                [this](td_api::authorizationStateWaitTdlibParameters &) {
                                    authState = AuthState::TDLIB_PARAMS;
                                    logger->named<TdManager>("AuthorizationStateWaitTdlibParameters entering");
                                    auto request = td_api::make_object<td_api::setTdlibParameters>();
                                    request->database_directory_ = "tdlib";
                                    request->use_message_database_ = true;
                                    request->use_secret_chats_ = true;
                                    request->api_id_ = API_ID;
                                    request->api_hash_ = API_HASH;
                                    request->system_language_code_ = "en";
                                    request->device_model_ = "Desktop";
                                    request->application_version_ = "1.0";
                                    string error;
                                    send_query(std::move(request), create_authentication_query_handler(&error));
                                }));
    }
    string check_authentication_error(Object object) {
        logger->named<TdManager>("check_authentication_error");
        if (object->get_id() == td_api::error::ID) {
            error = to_string(td::move_tl_object_as<td_api::error>(object));
            on_authorization_state_update();
            return error;
        }
        return "";
    }


    void process_response(td::ClientManager::Response response) {
        if (!response.object) {
            logger->named<TdManager>("No response");
            return;
        }
        logger->named<TdManager>(response.request_id + " " + to_string(response.object));
        if (response.request_id == 0) {
            return process_update(std::move(response.object));
        }
        auto it = handlers_.find(response.request_id);
        if (it != handlers_.end()) {

        it->second(std::move(response.object));
        handlers_.erase(it);
        }
    }

    void process_update(td_api::object_ptr<td_api::Object> update) {
        td_api::downcast_call(
            *update, overloaded(
                [this](td_api::updateAuthorizationState &update_authorization_state) {
                    authorization_state_ = std::move(update_authorization_state.authorization_state_);
                    on_authorization_state_update();
                },
                [this](td_api::updateNewChat &update_new_chat) {
                    chats_[update_new_chat.chat_->id_] = std::move(update_new_chat.chat_);
                    // chat_title_[update_new_chat.chat_->id_] = update_new_chat.chat_->title_;
                },
                [this](td_api::updateChatTitle &update_chat_title) {
                    chats_[update_chat_title.chat_id_]->title_ = update_chat_title.title_;
                },
                [this](td_api::updateUser &update_user) {
                    auto user_id = update_user.user_->id_;
                    // users_[user_id] = std::move(update_user.user_);
                },
                [this](td_api::updateNewMessage &update_new_message) {
                    auto chat_id = update_new_message.message_->chat_id_;
                    std::string sender_name;
                    // td_api::downcast_call(*update_new_message.message_->sender_id_,
                    //                         overloaded(
                    //                             [this, &sender_name](td_api::messageSenderUser &user) {
                    //                             sender_name = get_user_name(user.user_id_);
                    //                             },
                    //                             [this, &sender_name](td_api::messageSenderChat &chat) {
                    //                             sender_name = get_chat_title(chat.chat_id_);
                    //                             }));
                    std::string text;
                    if (update_new_message.message_->content_->get_id() == td_api::messageText::ID) {
                        text = static_cast<td_api::messageText &>(*update_new_message.message_->content_).text_->text_;
                    }
                    logger->named<TdManager>("Receive message: [chat_id:" + to_string(chat_id) + "] [from:" + sender_name + "] [" + text + "]");
                },
                [](auto &update) {}));
    }

    std::unique_ptr<td::ClientManager> client_manager_;
    std::int32_t client_id_{0};
    std::uint64_t current_query_id_{0};
    std::map<std::uint64_t, std::function<void(Object)>> handlers_;
    std::uint64_t authentication_query_id_{0};
    td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
    bool needRestart;
    bool idAuthed;
};