/**
 * @file IDAStarSolver.h
 * @brief Korf's IDA* solver guided by a Corner Pattern Database heuristic.
 *
 * IDA* (Iterative Deepening A*) combines the memory efficiency of IDDFS with
 * the directional intelligence of A*. Instead of iterating raw depth limits,
 * it iterates *f-cost thresholds* where f(n) = g(n) + h(n):
 *   - g(n) = actual moves taken so far (exact).
 *   - h(n) = lower-bound estimate of moves still needed (admissible heuristic).
 *
 * Any branch where f(n) > threshold is pruned entirely. Because h(n) never
 * over-estimates, the first solution found at the current threshold is
 * guaranteed to be optimal.
 *
 * @par The Heuristic — Corner Pattern Database
 * The database (corner_db.txt) maps every reachable 8-corner configuration
 * to the minimum number of moves needed to solve those corners alone. Since
 * solving the corners is a necessary sub-problem of solving the whole cube,
 * this count is always ≤ the true total distance — making it admissible.
 *
 * The database is built offline by CornerDBGenerator to depth
 * SolverConfig::CORNER_DB_DEPTH. States absent from the database are deeper
 * than that cutoff, so the heuristic returns SolverConfig::HEURISTIC_FALLBACK
 * (= CORNER_DB_DEPTH + 1) as a safe lower bound.
 *
 * @par Performance characteristics (13-move scramble, depth-6 DB)
 *   - DB load time:  15–45 seconds (once per run).
 *   - Solve time:    < 2 seconds for most 13-move scrambles.
 *
 * @par Template parameter
 *   IDAStarSolver operates on @c GenericRubiksCube& (the abstract interface),
 *   so it works with any concrete model. Using RubiksCubeBitboard gives the
 *   best performance due to its compact register-based state.
 */

#ifndef IDASTAR_SOLVER_H
#define IDASTAR_SOLVER_H

#include <vector>
#include <unordered_map>
#include <string>
#include "GenericRubiksCube.h"
#include "SolverConfig.h"
#include "MoveUtils.h"

/**
 * @class IDAStarSolver
 * @brief Optimal Rubik's Cube solver implementing Korf's IDA* algorithm.
 */
class IDAStarSolver {
private:

    /**
     * @brief The Corner Pattern Database loaded into RAM.
     *
     * Maps a 24-character corner-state string (from GenericRubiksCube::getCorners())
     * to the minimum number of moves required to solve those corners from that state.
     *
     * Populated once in the constructor from a pre-generated text file.
     * Typical size: ~1.3M entries at depth 6, ~9.3M at depth 7.
     */
    std::unordered_map<std::string, int> corner_db;

    /**
     * @brief Queries the Corner Pattern Database for h(n).
     *
     * Extracts the corner string from @p cube and looks it up in corner_db.
     * If the state is in the database, returns the stored minimum move count.
     * If not (state requires > CORNER_DB_DEPTH moves to solve corners), returns
     * SolverConfig::HEURISTIC_FALLBACK as a safe admissible lower bound.
     *
     * @param cube The cube whose corner state we want to evaluate.
     * @return A lower bound on the number of moves needed to fully solve @p cube.
     */
    int heuristic(const GenericRubiksCube& cube) const;

    /**
     * @brief Recursive A*-guided depth-limited search.
     *
     * At each node, computes f(n) = g + h(n). If f > threshold, prunes the
     * branch immediately. Otherwise, applies all non-redundant moves and
     * recurses. On backtrack, undoes the move in place via MoveUtils::inverse().
     *
     * @param cube        The cube state being explored, mutated in place.
     * @param g           Cost so far: exact number of moves applied from the root.
     * @param threshold   Current f-cost ceiling for this IDA* iteration.
     * @param path        Accumulates the move sequence; entries are popped on backtrack.
     * @param last_move   The most recently applied move; used for same-face pruning.
     *                    Pass @c static_cast<GenericRubiksCube::Move>(-1) at the root.
     * @return @c true if a solved state with f ≤ threshold was found.
     */
    bool dfs(GenericRubiksCube& cube,
             int g,
             int threshold,
             std::vector<GenericRubiksCube::Move>& path,
             GenericRubiksCube::Move last_move);

public:

    /**
     * @brief Loads the Corner Pattern Database from disk into RAM.
     *
     * Reads @p db_filepath line by line; each line is expected to be:
     * @code
     *   <24-char corner string> <depth>
     * @endcode
     *
     * Throws @c std::runtime_error if the file cannot be opened, so the caller
     * can handle the failure gracefully rather than receiving a hard @c exit(1).
     *
     * @param db_filepath Path to the pre-generated corner_db.txt file.
     * @throws std::runtime_error if @p db_filepath cannot be opened.
     */
    explicit IDAStarSolver(const std::string& db_filepath);

    /**
     * @brief Finds the optimal solution for @p cube using IDA*.
     *
     * The initial threshold is set to heuristic(cube) rather than 1, skipping
     * all depth iterations the heuristic guarantees cannot contain a solution.
     *
     * @param cube      The scrambled cube to solve (mutated during search, but
     *                  restored to the original state on each iteration boundary).
     * @param max_depth Hard upper bound on solution depth before giving up.
     *                  Default is SolverConfig::IDA_MAX_DEPTH (= 20 = God's Number).
     * @return The optimal (shortest) move sequence that solves @p cube,
     *         or an empty vector if no solution is found within @p max_depth.
     */
    std::vector<GenericRubiksCube::Move> solve(
        GenericRubiksCube& cube,
        int max_depth = SolverConfig::IDA_MAX_DEPTH);
};

#endif // IDASTAR_SOLVER_H
