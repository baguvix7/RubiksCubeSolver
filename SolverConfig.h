/**
 * @file SolverConfig.h
 * @brief Centralised configuration constants shared across all solver components.
 *
 * This header is the single source of truth for all "magic numbers" in the
 * solver pipeline. Keeping these constants here eliminates the hidden coupling
 * that previously existed between CornerDBGenerator and IDAStarSolver, where
 * changing MAX_DEPTH in the generator without updating the heuristic fallback
 * in the solver would silently produce an inadmissible heuristic and cause
 * IDA* to miss optimal solutions.
 *
 * @usage
 *   - When rebuilding the database to a new depth, change ONLY corner_db_depth.
 *   - All dependent values (heuristic fallback, default solve depth) update
 *     automatically via constexpr arithmetic.
 *
 * @warning
 *   corner_db_depth > 8 will likely exhaust system RAM (hundreds of millions
 *   of states). The static_assert below enforces this limit at compile time.
 */

#ifndef SOLVER_CONFIG_H
#define SOLVER_CONFIG_H

#include <vector>
#include "GenericRubiksCube.h"

namespace SolverConfig {

    // -----------------------------------------------------------------------
    // Pattern Database settings
    // -----------------------------------------------------------------------

    /**
     * @brief The depth to which CornerDBGenerator performs its BFS.
     *
     * At depth 6  → ~1.3 million states,  fast generation (~30s), good pruning.
     * At depth 7  → ~9.3 million states,  ~5-10 min generation, better pruning.
     * At depth 8  → ~65  million states,  ~45-60 min, best pruning (may need 4GB+ RAM).
     *
     * This value MUST match what was used when corner_db.txt was generated.
     * If you regenerate the database, change this constant and recompile everything.
     */
    constexpr int CORNER_DB_DEPTH = 6;

    static_assert(CORNER_DB_DEPTH <= 8,
        "CORNER_DB_DEPTH > 8 may exhaust RAM. "
        "Comment out this assert only if you have verified available memory.");

    // -----------------------------------------------------------------------
    // Heuristic fallback
    // -----------------------------------------------------------------------

    /**
     * @brief The h(n) value returned when a corner state is absent from the DB.
     *
     * Admissibility guarantee: Any state NOT in the database requires strictly
     * MORE than CORNER_DB_DEPTH moves to solve its corners. Therefore returning
     * CORNER_DB_DEPTH + 1 is a safe lower bound — it never over-estimates the
     * true distance, preserving IDA*'s optimality guarantee.
     *
     * This is derived automatically. Do not hard-code 7 (or any other literal)
     * in IDAStarSolver.cpp.
     */
    constexpr int HEURISTIC_FALLBACK = CORNER_DB_DEPTH + 1;

    // -----------------------------------------------------------------------
    // Solve depth limits
    // -----------------------------------------------------------------------

    /**
     * @brief The maximum search depth IDA* will attempt before giving up.
     *
     * God's Number for the 3×3 Rubik's Cube is 20 (in half-turn metric).
     * Setting this above 20 wastes time — any valid cube is solvable in ≤20.
     */
    constexpr int IDA_MAX_DEPTH = 20;

    /**
     * @brief The maximum depth DFS/IDDFS will search before giving up.
     *
     * IDDFS is only practical for shallow scrambles (≤9 moves) before the
     * branching factor makes it impractically slow. A cap of 20 matches
     * God's Number but expect very long runtimes past depth ~10.
     */
    constexpr int IDDFS_MAX_DEPTH = 20;

    // -----------------------------------------------------------------------
    // Shared move list
    // -----------------------------------------------------------------------

    /**
     * @brief All 18 standard quarter-turn and half-turn moves.
     *
     * Defined once here to eliminate the verbatim duplication that previously
     * existed in every solver class and in CornerDBGenerator. Each solver
     * references this list instead of maintaining its own copy.
     *
     * Ordering follows the enum grouping in GenericRubiksCube.h:
     *   - Every triplet shares a face: (Clockwise, Prime, Half-turn).
     *   - This ordering is load-bearing for the inverse-move formula in
     *     MoveUtils::inverse(). Do not rearrange.
     */
    inline const std::vector<GenericRubiksCube::Move> ALL_MOVES = {
        GenericRubiksCube::Move::L,  GenericRubiksCube::Move::L_PRIME, GenericRubiksCube::Move::L2,
        GenericRubiksCube::Move::R,  GenericRubiksCube::Move::R_PRIME, GenericRubiksCube::Move::R2,
        GenericRubiksCube::Move::U,  GenericRubiksCube::Move::U_PRIME, GenericRubiksCube::Move::U2,
        GenericRubiksCube::Move::D,  GenericRubiksCube::Move::D_PRIME, GenericRubiksCube::Move::D2,
        GenericRubiksCube::Move::F,  GenericRubiksCube::Move::F_PRIME, GenericRubiksCube::Move::F2,
        GenericRubiksCube::Move::B,  GenericRubiksCube::Move::B_PRIME, GenericRubiksCube::Move::B2
    };

} // namespace SolverConfig

#endif // SOLVER_CONFIG_H
