#pragma once

namespace state {
	// structs
  enum AuthState {
    TDLIB_PARAMS,
    PHONE_ENTER,
    CODE_ENTER,
    PASSWORD_ENTER,
    AUTHENTICATED,
		CLOSED,
  };

  enum ChangingAuthState {
    ENTERING,
    LOADING,
    ERROR,
  };

	// variables
  AuthState authState = AuthState::TDLIB_PARAMS;
  ChangingAuthState changeState = ChangingAuthState::ENTERING;
	bool needRestart = false;
};
