#pragma once
#include <ftxui/component/component.hpp>

#include <vector>
#include <memory>

#include <scenes/scene.h>
#include <utils/chats.h>

class SceneManager {
private:
	std::vector<std::shared_ptr<Scene>> scenes;
	std::shared_ptr<int> page;
	std::vector<Component> components;
public:
	SceneManager(ScreenInteractive& screen, ChatsManager* chats_manager);
	std::vector<std::shared_ptr<Scene>>& get_scenes();
	std::vector<Component>& get_components();
	Scene& get_current_scene();
	const std::shared_ptr<int> get_page();
	void update();
};

