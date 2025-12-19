#include <telegram/sender.h>

std::uint64_t Sender::next_query_id() {
	auto query_id = next_query_id();
}

void Sender::send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler) {
	handler_manager->add_handler(next_query_id(), handler);
  client_manager_->send(client_id_, current_query_id_, std::move(f));
}

