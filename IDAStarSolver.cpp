/**
 * @file IDAStarSolver.cpp
 * @brief IDA* search engine with Corner Pattern Database heuristic.
 *
 * Key changes from the original:
 *   - Constructor throws std::runtime_error on DB load failure instead of
 *     calling exit(1), preserving RAII cleanup for all live objects.
 *   - The heuristic fallback constant is derived from SolverConfig::HEURISTIC_FALLBACK
 *     (= CORNER_DB_DEPTH + 1) rather than being hard-coded as 7, eliminating
 *     the silent admissibility bug that would arise if the DB was ever regenerated
 *     to a different depth without updating this file.
 *   - Inverse-move calculation uses MoveUtils::inverse() (defined once, shared
 *     across all solvers) instead of the duplicated 8-line block that was here before.
 *   - Same-face pruning uses MoveUtils::isRedundant().
 *   - The move list is sourced from SolverConfig::ALL_MOVES.
 */

#include "IDAStarSolver.h"
#include "MoveUtils.h"
#include "SolverConfig.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

// =========================================================================
// Constructor — load Corner Pattern Database
// =========================================================================

IDAStarSolver::IDAStarSolver(const string& db_filepath) {
    cout << "[INFO] Loading Corner Pattern Database from: " << db_filepath << "\n";

    ifstream inFile(db_filepath);
    if (!inFile.is_open()) {
        // Throw instead of exit(1) so that RAII destructors run and the
        // caller can catch and handle the error gracefully.
        throw runtime_error(
            "[ERROR] Cannot open pattern database: '" + db_filepath + "'.\n"
            "        Run CornerDBGenerator first to produce corner_db.txt."
        );
    }

    string line, state;
    int depth;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        if (ss >> state >> depth) {
            corner_db[state] = depth;
        }
    }
    inFile.close();

    cout << "[INFO] Database loaded: " << corner_db.size() << " corner states in RAM.\n\n";
}

// =========================================================================
// heuristic — h(n)
// =========================================================================

int IDAStarSolver::heuristic(const GenericRubiksCube& cube) const {
    const string corners = cube.getCorners();

    auto it = corner_db.find(corners);
    if (it != corner_db.end()) {
        return it->second;  // Exact minimum moves for this corner configuration
    }

    // State not in database → it requires more than CORNER_DB_DEPTH moves to
    // solve the corners alone. HEURISTIC_FALLBACK = CORNER_DB_DEPTH + 1 is
    // the tightest admissible (non-overestimating) lower bound we can give.
    return SolverConfig::HEURISTIC_FALLBACK;
}

// =========================================================================
// dfs — A*-guided recursive search
// =========================================================================

bool IDAStarSolver::dfs(GenericRubiksCube& cube,
                         int g,
                         int threshold,
                         vector<GenericRubiksCube::Move>& path,
                         GenericRubiksCube::Move last_move) {

    const int h = heuristic(cube);
    const int f = g + h;

    // Prune: the estimated total cost already exceeds the current threshold.
    // No solution at or below `threshold` total moves can lie on this branch.
    if (f > threshold) return false;

    // Goal check: h == 0 means corners are solved; isSolved() confirms everything
    if (cube.isSolved()) return true;

    for (const auto& move : SolverConfig::ALL_MOVES) {

        if (MoveUtils::isRedundant(last_move, move)) continue;

        cube.performMove(move);
        path.push_back(move);

        // g increases by 1 (we just made a move); threshold stays the same
        if (dfs(cube, g + 1, threshold, path, move)) {
            return true;
        }

        path.pop_back();
        cube.performMove(MoveUtils::inverse(move));
    }

    return false;
}

// =========================================================================
// solve — IDA* outer loop
// =========================================================================

vector<GenericRubiksCube::Move> IDAStarSolver::solve(GenericRubiksCube& cube,
                                                       int max_depth) {
    vector<GenericRubiksCube::Move> path;

    // Start the threshold at h(root) rather than 1.
    // The heuristic guarantees no solution shorter than h(root) exists, so all
    // iterations below this starting value would be wasted work.
    const int start_threshold = heuristic(cube);

    for (int threshold = start_threshold; threshold <= max_depth; threshold++) {
        path.clear();

        if (dfs(cube, 0, threshold, path,
                static_cast<GenericRubiksCube::Move>(-1))) {
            return path;  // Optimal solution found
        }
    }

    // No solution within max_depth — should not happen for a valid cube
    return {};
}
