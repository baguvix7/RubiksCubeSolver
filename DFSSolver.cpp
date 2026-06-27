/**
 * @file DFSSolver.cpp
 * @brief Single-pass depth-limited DFS implementation.
 *
 * The structure is identical to one iteration of IDDFSSolver::dfs(), but
 * without the outer iterative-deepening loop. This means it searches to
 * exactly max_depth — it will miss solutions that exist at shallower depths.
 *
 * Backtracking uses MoveUtils::inverse() to undo each move in place,
 * keeping the cube mutation on a single shared object rather than copying.
 */

#include "DFSSolver.h"
#include "MoveUtils.h"
#include "SolverConfig.h"

using namespace std;

// =========================================================================
// dfs — recursive worker
// =========================================================================

bool DFSSolver::dfs(RubiksCube3dArray& cube,
                    int max_depth,
                    vector<GenericRubiksCube::Move>& path,
                    GenericRubiksCube::Move last_move) {

    if (cube.isSolved()) return true;
    if (max_depth == 0)  return false;

    for (const auto& move : SolverConfig::ALL_MOVES) {

        if (MoveUtils::isRedundant(last_move, move)) continue;

        cube.performMove(move);
        path.push_back(move);

        if (dfs(cube, max_depth - 1, path, move)) {
            return true;
        }

        path.pop_back();
        cube.performMove(MoveUtils::inverse(move));
    }

    return false;
}

// =========================================================================
// solve — single-pass entry point
// =========================================================================

vector<GenericRubiksCube::Move> DFSSolver::solve(RubiksCube3dArray cube,
                                                  int max_depth) {
    vector<GenericRubiksCube::Move> path;

    // Single pass directly to max_depth (no iterative deepening loop)
    dfs(cube, max_depth, path, static_cast<GenericRubiksCube::Move>(-1));

    return path;
}
