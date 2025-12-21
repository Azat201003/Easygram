#pragma once

#include <map>
#include <vector>
#include <td/telegram/td_api.h>

namespace td_api = td::td_api;

struct State {
enum AuthState {
  TDLIB_PARAMS,
  PHONE_ENTER,
  CODE_ENTER,
  PASSWORD_ENTER,
  AUTHENTICATED,
  CLOSED
};

enum ChangingAuthState {
  ENTERING,
  LOADING,
  ERROR
};

static inline AuthState authState = AuthState::TDLIB_PARAMS;
static inline ChangingAuthState changeState = ChangingAuthState::ENTERING;
static inline bool needRestart = false;
};

extern std::map<std::int64_t, td_api::object_ptr<td_api::chat>> chats;