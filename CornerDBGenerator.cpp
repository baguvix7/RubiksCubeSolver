/**
 * @file CornerDBGenerator.cpp
 * @brief Offline tool that generates the Corner Pattern Database for IDA*.
 *
 * @par What this does
 * Performs a Breadth-First Search backwards from the perfectly solved
 * RubiksCubeBitboard state, recording the minimum number of moves required
 * to reach each unique 8-corner configuration. The result is saved as a
 * plain-text file (corner_db.txt) that IDAStarSolver reads at runtime.
 *
 * @par Why BFS from the solved state?
 * BFS guarantees that the first time any corner state is discovered, it is
 * reached via the shortest path. Searching forwards from a scrambled state
 * would require a new BFS for every scramble; searching backwards from the
 * solved state builds the database once and makes it reusable for every scramble.
 *
 * @par Configuration
 * The search depth is controlled by SolverConfig::CORNER_DB_DEPTH (see SolverConfig.h).
 * A compile-time static_assert prevents accidentally setting depth > 8, which
 * would likely exhaust available RAM.
 *
 * Depth 6 → ~1.3 million states, ~30 seconds to generate, ~15 MB on disk.
 * Depth 7 → ~9.3 million states, ~5–10 minutes,            ~110 MB on disk.
 *
 * @par Output format
 * Each line of corner_db.txt is:
 * @code
 *   <24-char corner string> <depth>\n
 * @endcode
 * The corner string comes from RubiksCubeBitboard::getCorners(), which encodes
 * each sticker as the digit character of its face index ('0'–'5').
 *
 * @par Usage
 * @code
 *   g++ -O2 -std=c++17 CornerDBGenerator.cpp RubiksCubeBitboard.cpp -o gen
 *   ./gen
 * @endcode
 * Run once before the first IDA* solve. Re-run only if you change CORNER_DB_DEPTH.
 *
 * @warning
 * Do not change SolverConfig::CORNER_DB_DEPTH and then use an old corner_db.txt —
 * the heuristic in IDAStarSolver will silently become inadmissible.
 * Always regenerate the database after changing the depth constant.
 */

#include <iostream>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <string>
#include "RubiksCubeBitboard.h"
#include "SolverConfig.h"

using namespace std;

// =========================================================================
// BFS node — holds a Bitboard cube state and the depth at which it was found
// =========================================================================

/**
 * @struct BFSNode
 * @brief A single node in the backwards-BFS tree.
 */
struct BFSNode {
    RubiksCubeBitboard cube;  ///< The cube state at this node.
    int depth;                ///< Number of moves from the solved state to reach this node.
};

// =========================================================================
// main
// =========================================================================

int main() {
    cout << "================================================\n";
    cout << "   CORNER PATTERN DATABASE GENERATOR (Korf)    \n";
    cout << "================================================\n\n";

    // Compile-time guard: depth > 8 will likely exhaust system RAM.
    // This check is also in SolverConfig.h; it is repeated here as a
    // defence-in-depth reminder for anyone editing this file directly.
    static_assert(SolverConfig::CORNER_DB_DEPTH <= 8,
        "CORNER_DB_DEPTH > 8 may exhaust RAM. "
        "Confirm available memory before raising this limit.");

    const int MAX_DEPTH = SolverConfig::CORNER_DB_DEPTH;

    cout << "[CONFIG] Search depth : " << MAX_DEPTH << "\n";
    cout << "[CONFIG] Heuristic fallback will be : " << SolverConfig::HEURISTIC_FALLBACK << "\n\n";

    // -----------------------------------------------------------------------
    // 1. Initialise the database map and the BFS queue from the solved state
    // -----------------------------------------------------------------------

    unordered_map<string, int> database;
    queue<BFSNode> bfs_queue;

    RubiksCubeBitboard solved_cube;  // Default-constructed in solved state
    const string solved_corners = solved_cube.getCorners();

    database[solved_corners] = 0;
    bfs_queue.push({solved_cube, 0});

    int depth_tracker = 0;
    cout << "[BFS] Starting search...\n";
    cout << "[BFS] Depth 0 | 1 state (solved)\n";

    // -----------------------------------------------------------------------
    // 2. BFS loop
    // -----------------------------------------------------------------------

    while (!bfs_queue.empty()) {
        BFSNode current = bfs_queue.front();
        bfs_queue.pop();

        // Progress reporting: print a line each time we move to a new depth level
        if (current.depth > depth_tracker) {
            depth_tracker = current.depth;
            cout << "[BFS] Depth " << depth_tracker
                 << " | Database size: " << database.size() << " states\n";
        }

        // Do not expand nodes at the maximum depth limit
        if (current.depth == MAX_DEPTH) continue;

        // Expand: apply all 18 moves and record newly discovered corner states
        for (const auto& move : SolverConfig::ALL_MOVES) {
            RubiksCubeBitboard next_cube = current.cube;
            next_cube.performMove(move);

            const string corner_key = next_cube.getCorners();

            // Only record a state the first time it is seen (BFS guarantees
            // this is also the time it is reached via the shortest path).
            if (database.find(corner_key) == database.end()) {
                const int next_depth = current.depth + 1;
                database[corner_key] = next_depth;
                bfs_queue.push({next_cube, next_depth});
            }
        }
    }

    // -----------------------------------------------------------------------
    // 3. Write the database to disk
    // -----------------------------------------------------------------------

    const string output_path = "corner_db.txt";
    cout << "\n[SAVE] Writing " << database.size()
         << " states to " << output_path << " ...\n";

    ofstream out_file(output_path);
    if (!out_file.is_open()) {
        cerr << "[ERROR] Could not open " << output_path << " for writing.\n";
        return 1;
    }

    for (const auto& [state, depth] : database) {
        out_file << state << " " << depth << "\n";
    }
    out_file.close();

    cout << "[SUCCESS] Database saved to " << output_path << ".\n";
    cout << "          Re-run IDA* solver — it will load this file automatically.\n\n";

    return 0;
}
