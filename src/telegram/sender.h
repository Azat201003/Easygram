#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <map>
#include <functional>

namespace td_api = td::td_api;

class Sender {
private:
  using Object = td_api::object_ptr<td_api::Object>;
	std::int32_t client_id_{0};
	std::uint64_t current_query_id_{0};
	std::unique_ptr<td::ClientManager> client_manager_;
public:
	std::map<std::uint64_t, std::function<void(Object)>> handlers_;
	std::uint64_t next_query_id() { return ++current_query_id_; }
  void send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler);
};
