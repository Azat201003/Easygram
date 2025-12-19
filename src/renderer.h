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
#include <scenes/code.h>
#include <scenes/load.h>
#include <scenes/main.h>
#include <scenes/password.h>
#include <scenes/phone.h>
#include <scenes/scene.h>

using namespace ftxui;

Component getRenderer(ScreenInteractive &screen, Logger *logger);
