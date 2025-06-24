#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

class Scene {
private:
    Scene();
protected:
    SceneManager* sceneManager;
public:
    Scene(SceneManager* sceneManager) {this->sceneManager = sceneManager;}
    virtual std::function<Element()> getRender() = 0;
    virtual Element getElement() = 0;
};

class SceneManager {
private:
    SceneManager(Scene* currentScene, ScreenInteractive* screen);
public:
    SceneManager();
    virtual Element getElement() = 0;

    void start() {
        screen->Loop(Renderer(component, currentScene->getRender()));
    }
    ftxui::Component component; 
    ScreenInteractive* screen;
    Scene* currentScene;
};

class GetPhoneScene : public Scene {
    std::string phone;

    Element getElement() override {
        Component input_field = Input(&phone, "Input your phone: ");
        
        Component continue_button = Button("Continue", [&] {});

        Component quit_button = Button("Quit", sceneManager->screen->ExitLoopClosure());
        auto container = Container::Vertical({
            input_field,
            continue_button,
            quit_button
        });
    }

    std::function<Element()> getRender() override {
        return [&] {
            return vbox({

            });
        };
    }
};
