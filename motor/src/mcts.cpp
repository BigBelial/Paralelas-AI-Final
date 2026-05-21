#include "mcts.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <vector>
#include <omp.h>

namespace mancala {

namespace {

struct Node {
    Board state;
    int move_from_parent = -1;     // movimiento que llevó a este nodo
    int parent_side = -1;          // lado que MOVIÓ para llegar a este nodo
    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
    std::vector<int> untried_moves;
    std::int64_t visits = 0;
    double wins = 0.0;             // victorias desde la perspectiva de parent_side
};

inline double uct_value(const Node& child, double parent_log_visits, double c) {
    const double exploitation = child.wins / static_cast<double>(child.visits);
    const double exploration = c * std::sqrt(parent_log_visits / static_cast<double>(child.visits));
    return exploitation + exploration;
}

Node* select_uct(Node* node, double c) {
    while (node->untried_moves.empty() && !node->children.empty()) {
        const double log_n = std::log(static_cast<double>(node->visits));
        Node* best = nullptr;
        double best_v = -std::numeric_limits<double>::infinity();
        for (auto& ch : node->children) {
            const double v = uct_value(*ch, log_n, c);
            if (v > best_v) { best_v = v; best = ch.get(); }
        }
        node = best;
    }
    return node;
}

Node* expand(Node* node, std::mt19937_64& rng) {
    if (node->untried_moves.empty()) return node;
    std::uniform_int_distribution<size_t> dist(0, node->untried_moves.size() - 1);
    const size_t i = dist(rng);
    const int move = node->untried_moves[i];
    node->untried_moves.erase(node->untried_moves.begin() + static_cast<std::ptrdiff_t>(i));

    auto child = std::make_unique<Node>();
    child->state = node->state;
    child->move_from_parent = move;
    child->parent_side = node->state.side_to_move;
    child->parent = node;
    child->state.apply_move(move);
    child->untried_moves = child->state.legal_moves();
    Node* raw = child.get();
    node->children.push_back(std::move(child));
    return raw;
}

// Rollout: jugar al azar hasta el final. Devuelve 1 si gana p0, -1 si p1, 0 empate.
int rollout(Board state, std::mt19937_64& rng) {
    while (!state.terminal()) {
        const auto moves = state.legal_moves();
        if (moves.empty()) break;
        std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
        state.apply_move(moves[dist(rng)]);
    }
    state.collect_remaining();
    return state.winner();
}

void backpropagate(Node* node, int rollout_winner) {
    // rollout_winner: 1 si gana p0, -1 si p1, 0 empate.
    while (node != nullptr) {
        node->visits += 1;
        if (node->parent_side >= 0) {
            const int sign = (node->parent_side == 0) ? +1 : -1;
            const int score = rollout_winner * sign;  // +1 si parent_side ganó, -1 perdió, 0 empate
            if (score > 0) node->wins += 1.0;
            else if (score == 0) node->wins += 0.5;
        }
        node = node->parent;
    }
}

std::int64_t depth_sum_and_count(const Node* n, std::int64_t depth, std::int64_t& count) {
    std::int64_t sum = depth;
    count += 1;
    for (const auto& ch : n->children) sum += depth_sum_and_count(ch.get(), depth + 1, count);
    return sum;
}

// Búsqueda MCTS secuencial — devuelve un árbol con estadísticas de los hijos
// de la raíz (suficiente para combinar en root parallelization).
struct LocalResult {
    int best_move = -1;
    std::int64_t rollouts = 0;
    double avg_depth = 0.0;
    // Estadísticas por movimiento legal raíz, indexado igual que root.legal_moves()
    std::vector<int> root_moves;
    std::vector<std::int64_t> child_visits;
    std::vector<double> child_wins;
};

LocalResult run_serial(const Board& root_state, int simulations, double c, std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    Node root;
    root.state = root_state;
    root.untried_moves = root_state.legal_moves();

    for (int i = 0; i < simulations; ++i) {
        Node* leaf = select_uct(&root, c);
        Node* expanded = expand(leaf, rng);
        const int winner = rollout(expanded->state, rng);
        backpropagate(expanded, winner);
    }

    LocalResult lr;
    lr.rollouts = static_cast<std::int64_t>(simulations);

    std::int64_t count = 0;
    std::int64_t depth_sum = depth_sum_and_count(&root, 0, count);
    lr.avg_depth = count > 0 ? static_cast<double>(depth_sum) / static_cast<double>(count) : 0.0;

    lr.root_moves = root_state.legal_moves();
    lr.child_visits.assign(lr.root_moves.size(), 0);
    lr.child_wins.assign(lr.root_moves.size(), 0.0);
    for (const auto& ch : root.children) {
        auto it = std::find(lr.root_moves.begin(), lr.root_moves.end(), ch->move_from_parent);
        if (it == lr.root_moves.end()) continue;
        const size_t idx = static_cast<size_t>(it - lr.root_moves.begin());
        lr.child_visits[idx] = ch->visits;
        lr.child_wins[idx] = ch->wins;
    }
    return lr;
}

}  // namespace

MctsResult search_mcts(const Board& root_state, const MctsConfig& cfg) {
    MctsResult out;
    const auto root_moves = root_state.legal_moves();
    if (root_moves.empty()) return out;

    if (cfg.threads <= 1) {
        LocalResult lr = run_serial(root_state, cfg.simulations, cfg.c_uct, cfg.seed);
        // Mejor movimiento = más visitado (criterio MCTS estándar).
        std::int64_t best_v = -1;
        size_t best_i = 0;
        for (size_t i = 0; i < lr.root_moves.size(); ++i) {
            if (lr.child_visits[i] > best_v) { best_v = lr.child_visits[i]; best_i = i; }
        }
        out.move = lr.root_moves[best_i];
        out.rollouts = lr.rollouts;
        out.tree_depth_avg = lr.avg_depth;
        out.win_rate = best_v > 0
            ? lr.child_wins[best_i] / static_cast<double>(best_v)
            : 0.0;
        out.evaluation = out.win_rate;
        return out;
    }

    // --- Root parallelization con OpenMP ---
    // Cada hilo construye su propio árbol MCTS sobre la misma raíz; al
    // finalizar combinamos las estadísticas de los hijos de la raíz.
    // No requiere sincronización durante la búsqueda.
    const int T = cfg.threads;
    const int per_thread = cfg.simulations / T;
    const int remainder = cfg.simulations % T;

    std::vector<LocalResult> locals(T);

    #pragma omp parallel for num_threads(T) schedule(static)
    for (int t = 0; t < T; ++t) {
        const int sims = per_thread + (t < remainder ? 1 : 0);
        const std::uint64_t local_seed = cfg.seed ^ (0x9E3779B97F4A7C15ULL * static_cast<std::uint64_t>(t + 1));
        locals[t] = run_serial(root_state, sims, cfg.c_uct, local_seed);
    }

    // Combinar: sumar visits y wins por movimiento raíz.
    std::vector<std::int64_t> combined_visits(root_moves.size(), 0);
    std::vector<double> combined_wins(root_moves.size(), 0.0);
    double sum_depth = 0.0;
    std::int64_t total_rollouts = 0;
    for (const auto& lr : locals) {
        total_rollouts += lr.rollouts;
        sum_depth += lr.avg_depth;
        for (size_t i = 0; i < lr.root_moves.size(); ++i) {
            auto it = std::find(root_moves.begin(), root_moves.end(), lr.root_moves[i]);
            if (it == root_moves.end()) continue;
            const size_t k = static_cast<size_t>(it - root_moves.begin());
            combined_visits[k] += lr.child_visits[i];
            combined_wins[k] += lr.child_wins[i];
        }
    }

    std::int64_t best_v = -1;
    size_t best_i = 0;
    for (size_t i = 0; i < root_moves.size(); ++i) {
        if (combined_visits[i] > best_v) { best_v = combined_visits[i]; best_i = i; }
    }
    out.move = root_moves[best_i];
    out.rollouts = total_rollouts;
    out.tree_depth_avg = sum_depth / static_cast<double>(T);
    out.win_rate = best_v > 0
        ? combined_wins[best_i] / static_cast<double>(best_v)
        : 0.0;
    out.evaluation = out.win_rate;
    return out;
}

}  // namespace mancala
