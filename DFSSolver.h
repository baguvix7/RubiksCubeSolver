/**
 * @file DFSSolver.h
 * @brief Depth-First Search solver with a fixed depth ceiling.
 *
 * Unlike IDDFSSolver, this solver dives straight to @p max_depth in a single
 * pass without iterating. It is therefore NOT guaranteed to find the optimal
 * (shortest) solution — it finds *a* solution at exactly the given depth, or
 * reports failure if none exists at that depth.
 *
 * @par When to use DFS over IDDFS
 *   - When you only care that a solution exists, not that it is shortest.
 *   - When you already know (or can bound) the exact solution depth.
 *   - For stress-testing move correctness at a specific depth without the
 *     overhead of iterating from depth 1.
 *
 * @par Relationship to IDDFSSolver
 *   IDDFSSolver calls a structurally identical DFS internally but wraps it in
 *   a loop that raises the depth ceiling until a solution is found, guaranteeing
 *   optimality. DFSSolver skips that loop — it is essentially one iteration of
 *   IDDFS at a chosen depth.
 *
 * @par Pruning
 *   Same-face move pruning (via MoveUtils::isRedundant()) is applied to reduce
 *   the effective branching factor from 18 to ~13.
 */

#ifndef DFS_SOLVER_H
#define DFS_SOLVER_H

#include <vector>
#include "GenericRubiksCube.h"
#include "RubiksCube3dArray.h"
#include "SolverConfig.h"
#include "MoveUtils.h"

/**
 * @class DFSSolver
 * @brief Single-pass depth-bounded DFS — finds a solution, not necessarily the shortest.
 */
class DFSSolver {
private:

    /**
     * @brief Recursive DFS worker.
     *
     * Explores the move tree down to @p max_depth, pruning same-face sequences
     * and backtracking by applying the inverse move in place.
     *
     * @param cube      The cube being explored, mutated in place.
     * @param max_depth Remaining depth budget.
     * @param path      Accumulates the move sequence; entries are popped on backtrack.
     * @param last_move The most recently applied move; used for redundancy pruning.
     *                  Pass @c static_cast<GenericRubiksCube::Move>(-1) at the root.
     * @return @c true if the solved state was reached within the remaining budget.
     */
    bool dfs(RubiksCube3dArray& cube,
             int max_depth,
             std::vector<GenericRubiksCube::Move>& path,
             GenericRubiksCube::Move last_move);

public:

    /**
     * @brief Runs a single-pass DFS to @p max_depth and returns the first solution found.
     *
     * The cube is passed by value so DFSSolver does not modify the caller's object.
     *
     * @param cube      The scrambled cube to solve (copied on entry).
     * @param max_depth The exact depth to search. Default is SolverConfig::IDDFS_MAX_DEPTH.
     * @return A (not necessarily optimal) move sequence that solves @p cube,
     *         or an empty vector if no solution exists at this exact depth.
     */
    std::vector<GenericRubiksCube::Move> solve(
        RubiksCube3dArray cube,
        int max_depth = SolverConfig::IDDFS_MAX_DEPTH);
};

#endif // DFS_SOLVER_H
