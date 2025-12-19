#include "handlers.h"

std::uint64_t HandlerManager::next_query_id() {
	return ++current_query_id_;
}

void HandlerManager::addHandler(std::function<void(Object)> handler) {
  auto query_id = next_query_id();
	if (handler)	
		handlers_[id] = handler;
}

void HandlerManager::process(std::uint64_t id, Object obj) {
    auto it = handlers_.find(id);
    if (it != handlers_.end()) {
        it->second(std::move(obj));
        handlers_.erase(it);
    }
}
