#include <map>
#include <functional>


class HandlerManager {
private:
	std::map<std::uint64_t, std::function<void(Object)>> handlers_;
public:
	void addHandler(function<void(Object)>);
	void proceed(std::uint64_t id);
};

