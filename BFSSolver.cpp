/**
 * @file BFSSolver.cpp
 * @brief BFS implementation with memory-efficient path reconstruction.
 *
 * Key change from the original: instead of storing a growing
 * std::vector<Move> inside every queued node (O(depth) per node), we store
 * only an integer parent index. The full path is reconstructed in O(depth)
 * time once the goal is found, at the cost of one extra pass over the node
 * store. Total memory is now O(nodes) rather than O(depth × nodes).
 */

#include "BFSSolver.h"
#include "SolverConfig.h"
#include <queue>
#include <unordered_set>
#include <string>
#include <algorithm>

using namespace std;

// =========================================================================
// reconstructPath
// =========================================================================

vector<GenericRubiksCube::Move> BFSSolver::reconstructPath(int solved_index) const {
    vector<GenericRubiksCube::Move> path;

    // Walk the parent chain from the goal back to the root
    int idx = solved_index;
    while (node_store[idx].parent_index != -1) {
        path.push_back(node_store[idx].move_taken);
        idx = node_store[idx].parent_index;
    }

    // The path was collected in reverse (goal → root); flip it
    reverse(path.begin(), path.end());
    return path;
}

// =========================================================================
// solve
// =========================================================================

vector<GenericRubiksCube::Move> BFSSolver::solve(RubiksCube3dArray cube) {
    // Early exit: already solved
    if (cube.isSolved()) return {};

    // Clear any state from a previous solve call
    node_store.clear();

    // Frontier: holds indices into node_store, not the nodes themselves
    queue<int> frontier;
    unordered_set<string> visited;

    // Push the root node (no parent, sentinel move value -1)
    node_store.push_back({
        cube,
        -1,
        static_cast<GenericRubiksCube::Move>(-1)
    });
    frontier.push(0);
    visited.insert(cube.getStateString());

    while (!frontier.empty()) {
        const int current_idx = frontier.front();
        frontier.pop();

        // Expand: try all 18 moves from the current state
        for (const auto& move : SolverConfig::ALL_MOVES) {
            RubiksCube3dArray next_cube = node_store[current_idx].cube;
            next_cube.performMove(move);

            const string state_str = next_cube.getStateString();

            if (visited.count(state_str)) continue; // Already seen this state

            visited.insert(state_str);

            // Append the new node to the store and record its frontier position
            const int new_idx = static_cast<int>(node_store.size());
            node_store.push_back({next_cube, current_idx, move});

            if (next_cube.isSolved()) {
                return reconstructPath(new_idx);
            }

            frontier.push(new_idx);
        }
    }

    // Unreachable on any valid Rubik's Cube state (all states are solvable)
    return {};
}
