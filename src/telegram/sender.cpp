#include <telegram/sender.h>

Sender::Sender(Logger* logger_, HandlerManager* handler_manager_, std::unique_ptr<td::ClientManager> client_manager) {
	this->client_manager_ = std::move(client_manager);
	client_id_ = client_manager_->create_client_id();
	send_query(td_api::make_object<td_api::getOption>("version"), {});

	this->logger = logger_;
	this->handler_manager = handler_manager_;
}

std::uint64_t Sender::next_query_id() {
	return ++current_query_id_;
}

void Sender::send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler) {
	std::uint64_t query_id = next_query_id();
	handler_manager->add_handler(query_id, handler);
   client_manager_->send(client_id_, query_id, std::move(f));
}

