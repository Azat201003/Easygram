#include <telegram/sender.h>

TgSender::TgSender(Logger* logger_, HandlerManager* handler_manager_) {
	this->client_manager_ = td::ClientManager::get_manager_singleton();
	this->client_id_ = client_manager_->create_client_id();

	this->logger = logger_;
	this->handler_manager = handler_manager_;
}

std::uint64_t TgSender::next_query_id() {
	return ++current_query_id_;
}

void TgSender::send_query(td_api::object_ptr<td_api::Function> f,
                std::function<void(Object)> handler) {
	std::uint64_t query_id = next_query_id();
	handler_manager->add_handler(query_id, handler);
    client_manager_->send(client_id_, query_id, std::move(f));
	logger->named<TgSender>("send query");
}

