#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <logger/logger.h>

#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <fstream>
#include <thread>
#include <atomic>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <cmath>
#include <algorithm>

#include "telegram.cpp"

using namespace ftxui;
using namespace std;

class Scene {
protected:
    shared_ptr<int> page;
    Logger* logger;
    ScreenInteractive& screen;
public:
    Scene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : page(page), screen(screen), logger(logger) {}
    virtual ~Scene() = default;
    virtual Component getComponent() = 0;
    virtual Element getElement() = 0;
};


class LoadingScene : public Scene {
public:
    LoadingScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {}
    Component getComponent() override {
        return Container::Vertical({});
    }
    Element getElement() override {
        return vbox({
            text(" Loading... ") | center,
        });
    }
};

class PhoneScene : public Scene {
private:
    struct Components {
        Component input_field;
        Component continue_button;
        Component quit_button;
    };
    shared_ptr<Components> components;
    string phone;
    string error;
public:
    PhoneScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        components->input_field = Input(&phone, "Enter your phone");
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        components->continue_button = Button("Continue", [this] {
            TdManager::getInstance().setPhoneNumber(phone, &error);
        });
    }
    Component getComponent() override {
        return Container::Vertical({
            components->input_field,
            components->continue_button,
            components->quit_button
        });
    }
    Element getElement() override {
        return vbox({
            hbox(text(" Login telegram with phone: ") | bold, 
                components->input_field->Render()) | border | size(WIDTH, EQUAL, 50),
            filler(),
            hbox(components->continue_button->Render(), 
                text(" "), components->quit_button->Render()
            ),
            filler(),
            text(error) | color(Color::Red),
        });
    }
};


class CodeScene : public Scene {
private:
    struct Components {
        Component input_field;
        Component continue_button;
        Component quit_button;
    };
    shared_ptr<Components> components;
    string code;
    string error;
public:
    CodeScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        components->input_field = Input(&code, "Enter your code");
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        components->continue_button = Button("Continue", [this] {
            TdManager::getInstance().setCode(code, &error);
        });
    }
    Component getComponent() override {
        return Container::Vertical({
            components->input_field,
            components->continue_button,
            components->quit_button
        });
    }
    Element getElement() override {
        return vbox({
            hbox(text("Enter your verefication code: ") | bold, 
                components->input_field->Render()) | border | size(WIDTH, EQUAL, 50),
            filler(),
            hbox(components->continue_button->Render(), 
                text(" "), components->quit_button->Render()
            ),
            filler(),
            text(error) | color(Color::Red),
        });
    }
};

class PasswordScene : public Scene {
private:
    struct Components {
        Component input_field;
        Component continue_button;
        Component quit_button;
    };
    shared_ptr<Components> components;
    string password;
    string error;
public:
    PasswordScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        components->input_field = Input(&password, "Enter your password");
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        components->continue_button = Button("Continue", [this] {
            TdManager::getInstance().setPassword(password, &error);
        });
    }
    Component getComponent() override {
        return Container::Vertical({
            components->input_field,
            components->continue_button,
            components->quit_button
        });
    }
    Element getElement() override {
        return vbox({
            hbox(text(" Login password: ") | bold, 
                components->input_field->Render()) | border | size(WIDTH, EQUAL, 50),
            filler(),
            hbox(components->continue_button->Render(), 
                text(" "), components->quit_button->Render()
            ),
            filler(),
            text(error) | color(Color::Red),
        });
    }
};

class MainScene : public Scene {
private:
    struct Components {
        Component quit_button;
        Component chat_list;
        Component folders;
        Component chat;
    };
    shared_ptr<Components> components;
    vector<string> chat_titles;
    vector<string> visible_chat_titles;
    vector<string> folder_titles;
    int selected_chat = 0;
    int selected_visible_chat = 0;
    int selected_folder = 0;
    int chatMenuStart = 0;
    
    bool isArchived(const td_api::chat& chat) {
        for (const auto& chatList : chat.chat_lists_) {
            if (chatList && chatList->get_id() == td_api::chatListArchive::ID) {
                return true;
            }
        }
        return false;
    }

    void updateChatList() {
        chat_titles.clear();
        auto& tdManager = TdManager::getInstance();
        for (const auto& chat_pair : tdManager.chats_) {
            if (chat_pair.second && !isArchived(*chat_pair.second)) {
                chat_titles.push_back(chat_pair.second->title_);
            }
        }
        if (chat_titles.empty()) {
            chat_titles.push_back("No chats available");
        }

        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        visible_chat_titles.clear();

        for (int i = max(0, chatMenuStart); i < max(0, int(min(int(this->chat_titles.size()), chatMenuStart+w.ws_row-9))); i++) {
            visible_chat_titles.push_back(this->chat_titles.at(i));
        }
    }

    MenuOption createAutoscrolled() {
        shared_ptr<MenuOption> option = make_shared<MenuOption>();
        (*option) = MenuOption::VerticalAnimated();
        option->on_change = [this, option] {
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            // if ((chatMenuStart != 0 || selected_visible_chat != 0) && (chatMenuStart + w.ws_row - 11 != chat_titles.size() || selected_visible_chat != this->selected_chat + w.ws_row - 11)) {
                selected_chat = selected_visible_chat + this->chatMenuStart;
                // }
            int delta = max(0, max(min(this->chatMenuStart, this->selected_chat-1), this->selected_chat - w.ws_row + 11)) - this->chatMenuStart;
            this->chatMenuStart += delta;
            if (delta > 0) {
                selected_visible_chat -= 1;
            }
            if (delta < 0) {
                selected_visible_chat += 1;
            }
            this->logger->debug("this->selected_visible_chat - w.ws_row + 11 = " + to_string(this->selected_visible_chat - w.ws_row + 11));
            this->logger->debug("chatMenuStart = " + to_string(this->chatMenuStart) + string("\n\tint(this->chat_titles.size()) = ") + to_string(int(this->chat_titles.size())));
            // option->entries = {};
            this->logger->debug(to_string(max(0, int(min(int(this->chat_titles.size()), chatMenuStart+w.ws_row)))));
            // vector<string> entries;
            // option->entries = entries;
            // option->entries = vector<std::copy(chat_titles.begin() + chatMenuStart, chat_titles.begin()+chatMenuStart+w.ws_row, std::back_inserter(chat_titles))>;
        };
        return *option;
    }

public:
    MainScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        updateChatList();
        components->chat_list = Menu(&visible_chat_titles, &selected_visible_chat, createAutoscrolled());
        components->folders = Menu(&folder_titles, &selected_folder);
        // components->chat = Menu();
    }
    Component getComponent() override {
        return Container::Vertical({
            components->chat_list,
            components->quit_button,
            // components->chat,
        });
    }
    
    Element getElement() override {
        updateChatList();
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return hbox({
            vbox({
                text("chats") | bold | center,
                separator(),
                components->chat_list->Render() | border | size(HEIGHT, EQUAL, w.ws_row-10) | vscroll_indicator | flex,
                separator(),
                // hbox({
                //     text("Selected: ") | bold,
                //     text(selected_chat < chat_titles.size() ? chat_titles[selected_chat] : "None")
                // }),
                // filler(),
                components->quit_button->Render() | center | size(WIDTH, EQUAL, 200),
            }) | size(WIDTH, EQUAL, 30),
            vbox({

            }) | size(WIDTH, EQUAL, w.ws_col-30)
        }) | size(HEIGHT, EQUAL, w.ws_row);
    }
};

Component getRenderer(ScreenInteractive& screen, Logger* logger) {

    auto page = make_shared<int>(1);
    vector<shared_ptr<Scene>> scenes;

    scenes.push_back(make_shared<LoadingScene>(page, screen, logger));
    scenes.push_back(make_shared<PhoneScene>(page, screen, logger));
    scenes.push_back(make_shared<CodeScene>(page, screen, logger));
    scenes.push_back(make_shared<PasswordScene>(page, screen, logger));
    scenes.push_back(make_shared<MainScene>(page, screen, logger));
    
    vector<Component> components;
    for (auto& scene : scenes) {
        components.push_back(scene->getComponent());
    }
    auto container = Container::Tab(components, page.get());


    static std::atomic<bool> running(true);
    static std::thread worker([page] {
        TdManager& tdManager = TdManager::getInstance();
        while (running) {
            if (tdManager.changeState != TdManager::ChangingState::LOADING && tdManager.authState != TdManager::AuthState::AUTHENTICATED) {
                (*page) = tdManager.authState;
            } else if (tdManager.changeState == TdManager::ChangingState::LOADING) {
                (*page) = 0;
            } else {
                (*page) = 4;
            }
            tdManager.update_response();
            std::this_thread::sleep_for(100ms);
        }
    });

    return Renderer(container, [scenes, page] {
        return scenes.at(*page)->getElement() 
            | border
            | center;
    });
}
