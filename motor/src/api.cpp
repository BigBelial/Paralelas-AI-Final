#include "api.hpp"

#include <chrono>
#include <stdexcept>
#include <string>

#include "alphabeta.hpp"
#include "json_min.hpp"
#include "mcts.hpp"

namespace mancala::api {

namespace {

ApiResult err(int status, const std::string& msg) {
    jsonm::Writer w;
    w.begin_obj();
    w.key("error"); w.str(msg);
    w.end_obj();
    return {status, w.str_value()};
}

}  // namespace

std::string board_to_json(const Board& b) {
    jsonm::Writer w;
    w.begin_obj();
    w.key("board"); w.begin_arr();
    for (int i = 0; i < NUM_PITS; ++i) w.integer(b.pits[i]);
    w.end_arr();
    w.key("side"); w.integer(b.side_to_move);
    w.end_obj();
    return w.str_value();
}

ApiResult handle_move(const std::string& json_body) {
    Board b;
    std::string algo;
    int depth = -1;
    int simulations = -1;
    int threads = 1;
    double alpha_weight = 0.1;
    double c_uct = 1.41421356;

    try {
        jsonm::Parser p(json_body);
        p.expect('{');
        bool first = true;
        while (true) {
            p.skip_ws();
            if (p.consume('}')) break;
            if (!first) p.expect(',');
            first = false;
            std::string k = p.parse_string();
            p.expect(':');
            if (k == "board") {
                auto v = p.parse_int_array();
                if (v.size() != NUM_PITS) {
                    return err(422, "board debe tener 14 enteros");
                }
                for (int i = 0; i < NUM_PITS; ++i) {
                    if (v[i] < 0) return err(422, "board no puede tener negativos");
                    b.pits[i] = v[i];
                }
            } else if (k == "side") {
                int s = static_cast<int>(p.parse_number());
                if (s != 0 && s != 1) return err(422, "side debe ser 0 o 1");
                b.side_to_move = s;
            } else if (k == "algo") {
                algo = p.parse_string();
            } else if (k == "depth") {
                depth = static_cast<int>(p.parse_number());
            } else if (k == "simulations") {
                simulations = static_cast<int>(p.parse_number());
            } else if (k == "threads") {
                threads = static_cast<int>(p.parse_number());
            } else if (k == "alpha_weight") {
                alpha_weight = p.parse_number();
            } else if (k == "c_uct") {
                c_uct = p.parse_number();
            } else {
                // Campo desconocido: lo saltamos consumiendo el valor.
                p.skip_ws();
                // intento simple: string, número o array
                if (p.pos() < p.src().size() && p.src()[p.pos()] == '"') {
                    (void)p.parse_string();
                } else if (p.pos() < p.src().size() && p.src()[p.pos()] == '[') {
                    (void)p.parse_int_array();
                } else {
                    (void)p.parse_number();
                }
            }
        }
    } catch (const std::exception& e) {
        return err(422, std::string("JSON inválido: ") + e.what());
    }

    if (threads < 1) threads = 1;
    if (threads > 64) threads = 64;

    const auto t0 = std::chrono::steady_clock::now();

    if (algo == "alphabeta") {
        if (depth < 1) return err(400, "depth es obligatorio para alphabeta");
        AlphaBetaConfig cfg{depth, alpha_weight, threads};
        AlphaBetaResult r = search_alphabeta(b, cfg);
        const auto t1 = std::chrono::steady_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        jsonm::Writer w;
        w.begin_obj();
        w.key("move"); w.integer(r.move);
        w.key("evaluation"); w.integer(r.evaluation);
        w.key("elapsed_ms"); w.integer(ms);
        w.key("stats"); w.begin_obj();
            w.key("algo"); w.str("alphabeta");
            w.key("nodes"); w.integer(r.nodes);
            w.key("prunes"); w.integer(r.prunes);
        w.end_obj();
        w.key("threads_used"); w.integer(threads);
        w.end_obj();
        return {200, w.str_value()};
    }

    if (algo == "mcts") {
        if (simulations < 1) return err(400, "simulations es obligatorio para mcts");
        MctsConfig cfg;
        cfg.simulations = simulations;
        cfg.threads = threads;
        cfg.c_uct = c_uct;
        MctsResult r = search_mcts(b, cfg);
        const auto t1 = std::chrono::steady_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        jsonm::Writer w;
        w.begin_obj();
        w.key("move"); w.integer(r.move);
        w.key("evaluation"); w.number(r.evaluation);
        w.key("elapsed_ms"); w.integer(ms);
        w.key("stats"); w.begin_obj();
            w.key("algo"); w.str("mcts");
            w.key("rollouts"); w.integer(r.rollouts);
            w.key("tree_depth_avg"); w.number(r.tree_depth_avg);
            w.key("win_rate"); w.number(r.win_rate);
        w.end_obj();
        w.key("threads_used"); w.integer(threads);
        w.end_obj();
        return {200, w.str_value()};
    }

    return err(400, "algo debe ser 'alphabeta' o 'mcts'");
}

}  // namespace mancala::api
