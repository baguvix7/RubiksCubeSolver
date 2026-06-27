/**
 * @file BFSSolver.h
 * @brief Breadth-First Search solver operating on any GenericRubiksCube model.
 *
 * BFS explores the cube's state graph level by level, guaranteeing that the
 * first solution it finds is also the shortest possible sequence of moves.
 *
 * @par Complexity
 *   - Time:  O(18^d) where d is the solution depth.
 *   - Space: O(18^d) — BFS must hold the entire frontier in memory.
 *
 * @par When to use
 *   BFS is practical only for shallow scrambles (≤ 6–7 moves). Beyond that,
 *   memory usage becomes prohibitive. For deeper scrambles, use IDDFSSolver
 *   (optimal, O(1) memory) or IDAStarSolver (optimal + heuristic pruning).
 *
 * @par Memory optimisation vs original
 *   The original implementation stored a full path vector inside every queued
 *   node, causing O(depth × nodes) memory usage. This version stores only a
 *   parent index and the move taken, reconstructing the path by walking back
 *   through a flat node store after the solution is found.
 */

#ifndef BFS_SOLVER_H
#define BFS_SOLVER_H

#include <vector>
#include <queue>
#include <unordered_set>
#include <string>
#include "GenericRubiksCube.h"
#include "RubiksCube3dArray.h"
#include "SolverConfig.h"

/**
 * @class BFSSolver
 * @brief Optimal shortest-path solver using Breadth-First Search.
 */
class BFSSolver {
private:

    /**
     * @brief A node in the BFS graph.
     *
     * Stores the cube state, the index of the parent node (for path
     * reconstruction), and the move that was applied to reach this node.
     *
     * Using an integer parent index rather than a full path vector reduces
     * per-node memory from O(depth) to O(1), which is critical at depth 6+.
     */
    struct BFSNode {
        RubiksCube3dArray cube;               ///< Cube state at this node.
        int               parent_index;       ///< Index of the parent in node_store (-1 for root).
        GenericRubiksCube::Move move_taken;   ///< Move applied from parent to reach this node.
    };

    /**
     * @brief Flat store of every node created during the search.
     *
     * Nodes are appended in BFS order. The parent_index field of each node
     * points back into this store, enabling O(depth) path reconstruction
     * without storing the full path in every frontier node.
     */
    std::vector<BFSNode> node_store;

    /**
     * @brief Reconstructs the solution path by walking parent links.
     *
     * Starting from the solved node at @p solved_index, follows parent_index
     * links back to the root, then reverses the collected moves.
     *
     * @param solved_index Index of the goal node in node_store.
     * @return The move sequence from the initial state to the solved state.
     */
    std::vector<GenericRubiksCube::Move> reconstructPath(int solved_index) const;

public:

    /**
     * @brief Runs BFS from the given scrambled cube until the solved state is found.
     *
     * The cube is passed by value so BFSSolver does not modify the caller's object.
     * Internally, copies are made for each newly discovered state.
     *
     * @param cube The scrambled cube to solve (copied on entry).
     * @return The shortest sequence of moves that solves @p cube.
     *         Returns an empty vector if the cube is already solved.
     */
    std::vector<GenericRubiksCube::Move> solve(RubiksCube3dArray cube);
};

#endif // BFS_SOLVER_H
