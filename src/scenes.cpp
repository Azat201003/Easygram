#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <logger/logger.h>

#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <functional> // Добавлено для std::function
#include <fstream>
#include <thread>
#include <atomic>

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
private:
    bool phoneSetted = false;
public:
    string error;
    LoadingScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {}
    Component getComponent() override {
        return Container::Vertical({
            
        });
    }
    Element getElement() override {
        return vbox({
            text("Loading..."),
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
    bool phoneSetted = false;
    string error;
public:
    PhoneScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        // Инициализация поля ввода
        components->input_field = Input(&phone, "Enter your phone");
        // Кнопка выхода с действием
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
    bool phoneSetted = false;
public:
    CodeScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        // Инициализация поля ввода
        components->input_field = Input(&code, "Enter your code");
        // Кнопка выхода с действием
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
    string code;
    string error;
    bool phoneSetted = false;
public:
    PasswordScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        // Инициализация поля ввода
        components->input_field = Input(&code, "Enter your password");
        // Кнопка выхода с действием
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        components->continue_button = Button("Continue", [this] {
            TdManager::getInstance().setPassword(code, &error);
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
        vector<Component> chats;
    };
    shared_ptr<Components> components;
public:
    MainScene(shared_ptr<int> page, ScreenInteractive& screen, Logger* logger) : Scene(page, screen, logger) {
        components = make_shared<Components>();
        components->quit_button = Button("Quit", screen.ExitLoopClosure());
        // components->chats = ;
    }
    Component getComponent() override {
        return Container::Vertical({
            components->quit_button,
        });
    }
    Element getElement() override {
        return vbox({
            text("You are logged in") | color(Color::Green1),
            components->quit_button->Render(),
            filler(),
        });
    }
};

class Timer {
public:
    const int INTERVAL = 4;
    time_t last_call;
    Timer() {
        last_call = time(nullptr);
    }
    bool timeCome() {
        time_t current = time(nullptr);
        last_call = current;
        return current - last_call >= INTERVAL;
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
    // Timer* timer = static_cast<Timer*>(calloc(sizeof(Timer), 1));
        while (running) {
            if (tdManager.changeState != TdManager::ChangingState::LOADING && tdManager.authState != TdManager::AuthState::AUTHENTICATED) {
                (*page) = tdManager.authState;
            } else if (tdManager.changeState == TdManager::ChangingState::LOADING) {
                (*page) = 0; // loading scene
            } else {
                (*page) = 4;
            }
            tdManager.update_response();
            
            std::this_thread::sleep_for(100ms); // Интервал обновления
        }
    });

    return Renderer(container, [scenes, page] {
        return scenes.at(*page)->getElement() 
            | border 
            | size(HEIGHT, LESS_THAN, 10) 
            | center;
    });
}
