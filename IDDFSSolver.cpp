/**
 * @file IDDFSSolver.cpp
 * @brief Recursive DFS engine and iterative deepening manager for IDDFSSolver.
 *
 * Backtracking is done by applying MoveUtils::inverse() to undo the move
 * in place, avoiding any cube copies on the call stack. The path vector
 * is also maintained in place (push before recurse, pop on backtrack).
 *
 * Same-face pruning via MoveUtils::isRedundant() is applied before every
 * recursive call, cutting the effective branching factor from 18 to ~13.
 */

#include "IDDFSSolver.h"
#include "MoveUtils.h"
#include "SolverConfig.h"

using namespace std;

// =========================================================================
// dfs — recursive depth-limited search
// =========================================================================

bool IDDFSSolver::dfs(RubiksCube3dArray& cube,
                      int depth_limit,
                      vector<GenericRubiksCube::Move>& path,
                      GenericRubiksCube::Move last_move) {

    // Base case: depth budget exhausted — check if we landed on the solution
    if (depth_limit == 0) {
        return cube.isSolved();
    }

    for (const auto& move : SolverConfig::ALL_MOVES) {

        // Pruning: two consecutive moves on the same face always reduce to
        // a simpler single move or cancel entirely — skip them.
        if (MoveUtils::isRedundant(last_move, move)) continue;

        // Apply the move to the shared cube state (no copy)
        cube.performMove(move);
        path.push_back(move);

        if (dfs(cube, depth_limit - 1, path, move)) {
            return true;  // Solution found down this branch — propagate up
        }

        // Backtrack: remove the move from history and undo it on the cube
        path.pop_back();
        cube.performMove(MoveUtils::inverse(move));
    }

    // All branches at this level were dead ends
    return false;
}

// =========================================================================
// solve — iterative deepening manager
// =========================================================================

vector<GenericRubiksCube::Move> IDDFSSolver::solve(RubiksCube3dArray cube,
                                                    int max_depth) {
    vector<GenericRubiksCube::Move> path;

    // Iterate depth limits from 1 up to max_depth.
    // Each iteration runs a fresh DFS — the cube argument is passed by value
    // above so we get a clean copy to mutate per outer iteration.
    for (int depth = 1; depth <= max_depth; depth++) {
        // Reset the cube to its original scrambled state each iteration.
        // Because `cube` is a local copy (passed by value into solve()), we
        // need to track the original. Re-solve from scratch each pass.
        RubiksCube3dArray working_cube = cube; // take a fresh copy each pass
        path.clear();

        if (dfs(working_cube, depth, path,
                static_cast<GenericRubiksCube::Move>(-1))) {
            return path;  // Optimal solution found at this depth
        }
    }

    // No solution found within max_depth — cube may be in an invalid state
    return {};
}
