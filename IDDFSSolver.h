/**
 * @file IDDFSSolver.h
 * @brief Iterative Deepening Depth-First Search (IDDFS) solver.
 *
 * IDDFS combines the key strengths of its two parent algorithms:
 *   - From BFS: guaranteed optimal (shortest) solution.
 *   - From DFS: O(depth) memory usage — the call stack holds at most one path
 *     at a time, with no visited-set overhead.
 *
 * @par How it works
 * The algorithm runs a strict DFS to depth limit 1. If that fails, it wipes
 * its state and runs a fresh DFS to depth 2, then 3, and so on. Although this
 * re-explores the shallower levels on every iteration, the branching factor of
 * the Rubik's Cube (≈13 after pruning) means each new level contains far more
 * nodes than all previous levels combined, so the total re-work is bounded.
 *
 * @par Performance
 *   - Depth 7: typically < 1 second.
 *   - Depth 8: 1–10 seconds depending on the scramble.
 *   - Depth 9+: impractical without the IDA* heuristic.
 *
 * For deeper scrambles, use IDAStarSolver which uses a corner heuristic to
 * skip the first several depth iterations entirely.
 *
 * @par Pruning
 * Same-face move pruning (via MoveUtils::isRedundant()) reduces the effective
 * branching factor from 18 to approximately 13–15.
 */

#ifndef IDDFS_SOLVER_H
#define IDDFS_SOLVER_H

#include <vector>
#include "GenericRubiksCube.h"
#include "RubiksCube3dArray.h"
#include "SolverConfig.h"
#include "MoveUtils.h"

/**
 * @class IDDFSSolver
 * @brief Optimal, memory-efficient solver using Iterative Deepening DFS.
 */
class IDDFSSolver {
private:

    /**
     * @brief Recursive depth-limited DFS worker.
     *
     * Explores the move tree up to @p depth_limit levels deep. Uses in-place
     * mutation and explicit backtracking (MoveUtils::inverse()) to avoid
     * copying the cube state on every recursive call.
     *
     * @param cube        The cube state being explored, mutated in place.
     * @param depth_limit Maximum additional moves permitted from this node.
     * @param path        Accumulates the move sequence; entries are popped on backtrack.
     * @param last_move   The move most recently applied; used by MoveUtils::isRedundant()
     *                    to prune same-face follow-ups.
     *                    Pass @c static_cast<GenericRubiksCube::Move>(-1) at the root.
     * @return @c true if a solved state was reached within @p depth_limit moves.
     */
    bool dfs(RubiksCube3dArray& cube,
             int depth_limit,
             std::vector<GenericRubiksCube::Move>& path,
             GenericRubiksCube::Move last_move);

public:

    /**
     * @brief Finds the optimal (shortest) solution via iterative deepening.
     *
     * Runs successive DFS passes at depth limits 1, 2, 3, … up to @p max_depth.
     * Returns the solution from the first pass that succeeds.
     *
     * @param cube      The scrambled cube to solve (copied on entry).
     * @param max_depth Upper bound on solution length before giving up.
     *                  Default is SolverConfig::IDDFS_MAX_DEPTH (= 20 = God's Number).
     * @return The shortest move sequence that solves @p cube, or an empty
     *         vector if no solution exists within @p max_depth moves.
     */
    std::vector<GenericRubiksCube::Move> solve(
        RubiksCube3dArray cube,
        int max_depth = SolverConfig::IDDFS_MAX_DEPTH);
};

#endif // IDDFS_SOLVER_H
