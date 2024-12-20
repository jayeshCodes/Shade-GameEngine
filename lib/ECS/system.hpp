//
// Created by Utsav Lal on 10/2/24.
//

#pragma once
#include <set>
#include <mutex>
#include "types.hpp"

/**
 * Inherit this to define a system
 */
class System {
public:
    std::set<Entity> entities;
    std::mutex update_mutex;
};
