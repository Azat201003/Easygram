#pragma once

template<typename Derived>
class Singleton {
public:
	static Derived& getInstance() noexcept {
		static Derived instance;
		return instance;
	}
};
