#include <scenes/manager.h>
#include <scenes/load.h>
#include <scenes/phone.h>
#include <scenes/code.h>
#include <scenes/password.h>
#include <scenes/main.h>
#include <state.h>

SceneManager::SceneManager(ScreenInteractive& screen, ChatsManager* chats_manager) {
	page = std::make_shared<int>(1);
	scenes.push_back(std::make_shared<LoadingScene>(page, screen));
	scenes.push_back(std::make_shared<PhoneScene>(page, screen));
	scenes.push_back(std::make_shared<CodeScene>(page, screen));
  scenes.push_back(std::make_shared<PasswordScene>(page, screen));
  scenes.push_back(std::make_shared<MainScene>(page, screen, chats_manager));

	for (auto& s : scenes) components.push_back(s->getComponent());
}

const std::shared_ptr<int> SceneManager::get_page() {
	return page;
}

std::vector<std::shared_ptr<Scene>>& SceneManager::get_scenes() {
	return scenes;
}

Scene& SceneManager::get_current_scene() {
	return *scenes[*page];
}

std::vector<Component>& SceneManager::get_components() {
	return components;
}

void SceneManager::update() {
	if (State::changeState != State::ChangingAuthState::LOADING &&
			State::authState != State::AuthState::AUTHENTICATED) {
		(*page) = State::authState;
	} else if (State::changeState == State::ChangingAuthState::LOADING) {
		(*page) = 0;
	} else {
		(*page) = 4;
	}
	scenes[*page]->ping();
}

