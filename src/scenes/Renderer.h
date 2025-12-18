#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <logger/logger.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <telegram.cpp>
#include <scenes/CodeScene.h>
#include <scenes/LoadingScene.h>
#include <scenes/MainScene.h>
#include <scenes/PasswordScene.h>
#include <scenes/PhoneScene.h>
#include <scenes/Scene.h>

using namespace ftxui;

Component getRenderer(ScreenInteractive &screen, Logger *logger);
