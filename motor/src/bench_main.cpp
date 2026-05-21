// Modo benchmark del motor. Ejecuta el algoritmo elegido sobre un conjunto
// de posiciones (formato: una posición por línea, 14 enteros separados por
// espacios, seguidos del lado 0/1) y reporta:
//   - tiempo total T(p)
//   - speedup S(p) y eficiencia E(p) cuando se compara contra T(1)
//   - métricas específicas del algoritmo (nodos+podas o rollouts+depth_avg)
//
// Uso:
//   mancala_bench --algo alphabeta --depth 12 --threads 8 --positions tests/suite.txt
//   mancala_bench --algo mcts      --simulations 100000 --threads 8 --positions tests/suite.txt

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>

#include "alphabeta.hpp"
#include "board.hpp"
#include "mcts.hpp"

namespace {

struct Args {
    std::string algo = "alphabeta";
    int depth = 12;
    int simulations = 100000;
    int threads = 1;
    std::string positions_file = "tests/suite.txt";
    double alpha_weight = 0.1;
};

void usage(const char* prog) {
    std::cerr <<
        "Uso: " << prog << " --algo alphabeta|mcts [opciones]\n"
        "  --depth N           profundidad para alphabeta (def 12)\n"
        "  --simulations N     simulaciones para mcts (def 100000)\n"
        "  --threads N         hilos OpenMP (def 1)\n"
        "  --alpha-weight F    peso alpha de la heurística (def 0.1)\n"
        "  --positions FILE    archivo con posiciones (def tests/suite.txt)\n";
}

std::vector<mancala::Board> load_positions(const std::string& path) {
    std::vector<mancala::Board> out;
    std::ifstream in(path);
    if (!in) {
        std::cerr << "[bench] no se puede abrir " << path
                  << "; usando posición inicial.\n";
        out.push_back(mancala::Board::initial());
        return out;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        mancala::Board b;
        for (int i = 0; i < mancala::NUM_PITS; ++i) ss >> b.pits[i];
        ss >> b.side_to_move;
        if (ss.fail()) {
            std::cerr << "[bench] línea inválida ignorada: " << line << "\n";
            continue;
        }
        out.push_back(b);
    }
    if (out.empty()) out.push_back(mancala::Board::initial());
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string k = argv[i];
        auto next = [&](int& dst) { if (i + 1 < argc) dst = std::atoi(argv[++i]); };
        auto next_d = [&](double& dst) { if (i + 1 < argc) dst = std::atof(argv[++i]); };
        auto next_s = [&](std::string& dst) { if (i + 1 < argc) dst = argv[++i]; };
        if (k == "--algo") next_s(a.algo);
        else if (k == "--depth") next(a.depth);
        else if (k == "--simulations") next(a.simulations);
        else if (k == "--threads") next(a.threads);
        else if (k == "--alpha-weight") next_d(a.alpha_weight);
        else if (k == "--positions") next_s(a.positions_file);
        else if (k == "-h" || k == "--help") { usage(argv[0]); return 0; }
        else { std::cerr << "argumento desconocido: " << k << "\n"; usage(argv[0]); return 2; }
    }

    if (a.threads < 1) a.threads = 1;
    omp_set_num_threads(a.threads);

    auto positions = load_positions(a.positions_file);

    std::cout << "[bench] algo=" << a.algo
              << " threads=" << a.threads
              << " positions=" << positions.size() << "\n";

    const double t0 = omp_get_wtime();

    std::int64_t total_nodes = 0, total_prunes = 0;
    std::int64_t total_rollouts = 0;
    double depth_sum = 0.0;

    for (const auto& pos : positions) {
        if (a.algo == "alphabeta") {
            mancala::AlphaBetaConfig cfg{a.depth, a.alpha_weight, a.threads};
            auto r = mancala::search_alphabeta(pos, cfg);
            total_nodes += r.nodes;
            total_prunes += r.prunes;
        } else if (a.algo == "mcts") {
            mancala::MctsConfig cfg;
            cfg.simulations = a.simulations;
            cfg.threads = a.threads;
            auto r = mancala::search_mcts(pos, cfg);
            total_rollouts += r.rollouts;
            depth_sum += r.tree_depth_avg;
        } else {
            std::cerr << "algo desconocido: " << a.algo << "\n";
            return 2;
        }
    }

    const double t1 = omp_get_wtime();
    const double elapsed = t1 - t0;

    std::cout << "[bench] T(" << a.threads << ") = " << elapsed << " s\n";
    if (a.algo == "alphabeta") {
        std::cout << "[bench] depth=" << a.depth
                  << " nodes_total=" << total_nodes
                  << " prunes_total=" << total_prunes << "\n";
    } else {
        std::cout << "[bench] simulations=" << a.simulations
                  << " rollouts_total=" << total_rollouts
                  << " tree_depth_avg_mean=" << (depth_sum / positions.size()) << "\n";
    }

    // Pista para análisis: dejar la línea final en CSV fácil de parsear.
    std::cout << "CSV,"
              << a.algo << ","
              << a.threads << ","
              << (a.algo == "alphabeta" ? a.depth : a.simulations) << ","
              << elapsed << ","
              << total_nodes << ","
              << total_prunes << ","
              << total_rollouts << "\n";

    return 0;
}
