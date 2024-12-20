#pragma once
#include <SDL.h>
#include "timeline.hpp"
#include <mutex>
#include <atomic>

extern std::atomic<bool> gameRunning;
extern std::mutex runningMutex;

void doInput();

SDL_FPoint getKeyPress();

void temporalInput(Timeline &gameTimeline);