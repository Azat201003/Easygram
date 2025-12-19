#include "handlers.h"

void HandlerManager::add_handler(std::uint64_t id, std::function<void(Object)> handler) {
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
