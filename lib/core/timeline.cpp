//
// Created by Jayesh Gajbhar on 9/10/24.
//

#include "timeline.hpp"

Timeline::Timeline(Timeline *anchor, int64_t tic)
    : start_time(anchor ? anchor->getTime() : 0), // If there is an anchor, use its time; otherwise start from 0
      elapsed_paused_time(0), paused(false), anchor(anchor), tic(tic) {
}

int64_t Timeline::currentSystemTime() {
    using namespace std::chrono;
    // get current system time
    auto now = system_clock::now();
    // convert it to milliseconds
    auto now_ms = duration_cast<milliseconds>(now.time_since_epoch());
    return now_ms.count();
}


void Timeline::start() {
    std::lock_guard<std::mutex> lock(m);
    // Use anchor's current time, not its start time
    start_time = anchor ? anchor->getTime() : currentSystemTime();
    elapsed_paused_time = 0;
    paused = false;
}

int64_t Timeline::getTime() {
    std::lock_guard<std::mutex> lock(m);

    if (paused) {
        return last_paused_time; // If paused, return the time when it was last paused
    }

    // Get the time from the anchor if it exists, otherwise use the system time
    int64_t current_time = anchor ? anchor->getTime() : currentSystemTime();

    // Return the adjusted time based on start time and tic
    if (tic == 0) tic = 1;  // Ensure tic is never zero
    return (current_time - start_time - elapsed_paused_time) / tic;
}

void Timeline::pause() {
    std::lock_guard<std::mutex> lock(m);

    if (!paused) {
        last_paused_time = getTime(); // Record the time of pause
        paused = true;
    }
}

void Timeline::unpause() {
    std::lock_guard<std::mutex> lock(m);

    if (paused) {
        int64_t current_time = anchor ? anchor->getTime() : 0;
        elapsed_paused_time += (current_time - last_paused_time); // Update paused time
        paused = false;
    }
}

void Timeline::changeTic(float new_tic) {
    std::lock_guard<std::mutex> lock(m);

    // Update current time according to old tic before changing
    int64_t current_time = getTime();
    tic = new_tic;

    // Re-adjust start_time based on the new tic
    start_time = anchor ? anchor->getTime() - (current_time / tic) : 0;
}

bool Timeline::isPaused() {
    std::lock_guard<std::mutex> lock(m);
    return paused;
}

float Timeline::getTic() {
    return tic;
}
