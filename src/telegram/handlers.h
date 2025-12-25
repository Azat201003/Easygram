#pragma once
#include <map>
#include <functional>
#include <cstdint>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <logger/logger.h>

using Object = td::td_api::object_ptr<td::td_api::Object>;

// handlers for response on requests
class HandlerManager {
private:
	std::map<std::uint64_t, std::function<void(Object)>> handlers_;
	Logger* logger;
public:
	HandlerManager();
	void add_handler(std::uint64_t id, std::function<void(Object)> handler);
	void process(std::uint64_t id, Object obj);
};

