#pragma once

#include <cstdint>

#include "board.hpp"

namespace mancala {

struct MctsResult {
    int move = -1;
    double evaluation = 0.0;   // win-rate del mejor hijo desde la raíz
    std::int64_t rollouts = 0;
    double tree_depth_avg = 0.0;
    double win_rate = 0.0;
};

struct MctsConfig {
    int simulations = 10000;
    double c_uct = 1.41421356;  // sqrt(2)
    int threads = 1;            // 1 == secuencial; >1 activa root parallelization
    std::uint64_t seed = 0xC0FFEE;
};

MctsResult search_mcts(const Board& root, const MctsConfig& cfg);

}  // namespace mancala
