#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <logger/logger.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "../telegram.cpp"
#include "CodeScene.h"
#include "LoadingScene.h"
#include "MainScene.h"
#include "PasswordScene.h"
#include "PhoneScene.h"
#include "Scene.h"

using namespace ftxui;

Component getRenderer(ScreenInteractive &screen, Logger *logger);