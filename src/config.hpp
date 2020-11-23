//
// Created by grant on 11/19/20.
//

#pragma once

#ifndef NEWTONIAN_FOOTBALL_2D_CONFIG_HPP
#define NEWTONIAN_FOOTBALL_2D_CONFIG_HPP

#define ENABLE_LOGGING


constexpr int threadPoolConvarTimeoutMs = 1000;

constexpr bool logToStdout = true;
constexpr auto logToLatestLog = false;
constexpr auto logToUniqueFile = false;

constexpr auto versionString = "v0.0.1";

constexpr char logsDir[] = "logs";

constexpr auto fieldHeight = 480;
constexpr auto fieldWidth = 300;
constexpr auto wallWidth = 1;
constexpr auto fbuf = 10;

constexpr auto targetFps = 0; // set to 0 for vsync, -1 for unlimited


#endif //NEWTONIAN_FOOTBALL_2D_CONFIG_HPP
