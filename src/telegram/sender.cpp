#include <telegram/sender.h>

void Sender::send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler) {
	auto query_id = next_query_id();
	if (handler) {
    handlers_.emplace(query_id, std::move(handler));
  }
  client_manager_->send(client_id_, query_id, std::move(f));
}

