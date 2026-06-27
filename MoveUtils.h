/**
 * @file MoveUtils.h
 * @brief Utility functions for Rubik's Cube move manipulation.
 *
 * This header consolidates move-related helper logic that was previously
 * duplicated verbatim across IDDFSSolver, DFSSolver, and IDAStarSolver.
 * Centralising these functions here ensures consistent behaviour and makes
 * future changes (e.g. adding new move types) a single-point edit.
 */

#ifndef MOVE_UTILS_H
#define MOVE_UTILS_H

#include "GenericRubiksCube.h"

namespace MoveUtils {

    /**
     * @brief Computes the mathematical inverse of a Rubik's Cube move.
     *
     * Relies on the move enum being organised in contiguous triplets per face:
     *   - Index % 3 == 0 : Clockwise 90°       → inverse is Prime (index + 1)
     *   - Index % 3 == 1 : Counter-clockwise 90° → inverse is Clockwise (index - 1, i.e. base)
     *   - Index % 3 == 2 : Half-turn 180°        → inverse is itself (self-inverse)
     *
     * @warning This formula is tightly coupled to the Move enum ordering in
     *          GenericRubiksCube.h. If the enum is ever reordered, this function
     *          must be updated to match.
     *
     * @param move The move to invert.
     * @return The move that exactly undoes @p move.
     */
    inline GenericRubiksCube::Move inverse(GenericRubiksCube::Move move) {
        const int val       = static_cast<int>(move);
        const int face_base = (val / 3) * 3;  // Base clockwise move for this face
        const int move_type = val % 3;         // 0=CW, 1=CCW, 2=Half

        if (move_type == 0) {
            // Clockwise → undo with Prime (face_base + 1)
            return static_cast<GenericRubiksCube::Move>(face_base + 1);
        }
        if (move_type == 1) {
            // Prime → undo with Clockwise (face_base + 0)
            return static_cast<GenericRubiksCube::Move>(face_base);
        }
        // Half-turn → undo with another half-turn (self-inverse)
        return move;
    }

    /**
     * @brief Determines whether applying @p current after @p last is redundant.
     *
     * Two consecutive moves on the same face always combine into a simpler
     * equivalent or cancel out entirely (e.g. U followed by U' is a no-op;
     * U followed by U2 equals U'). Pruning these branches does not affect
     * solution correctness but significantly reduces the search space.
     *
     * @param last    The most recently applied move.
     *                Pass static_cast<GenericRubiksCube::Move>(-1) at the root
     *                node to indicate no prior move exists.
     * @param current The candidate move being evaluated.
     * @return true if @p current should be skipped (same face as @p last).
     */
    inline bool isRedundant(GenericRubiksCube::Move last,
                            GenericRubiksCube::Move current) {
        // -1 sentinel: root node, no prior move — nothing to prune
        if (static_cast<int>(last) == -1) return false;

        // Moves on the same face share the same integer-divided face group.
        // e.g. L=0, L_PRIME=1, L2=2 all divide to group 0.
        const int last_face    = static_cast<int>(last)    / 3;
        const int current_face = static_cast<int>(current) / 3;

        return last_face == current_face;
    }

    /**
     * @brief Returns a human-readable string for a move (useful for debugging).
     *
     * @param move The move to stringify.
     * @return A short SiGN-notation string such as "U", "R'", "F2".
     */
    inline const char* toString(GenericRubiksCube::Move move) {
        switch (move) {
            case GenericRubiksCube::Move::L:       return "L";
            case GenericRubiksCube::Move::L_PRIME: return "L'";
            case GenericRubiksCube::Move::L2:      return "L2";
            case GenericRubiksCube::Move::R:       return "R";
            case GenericRubiksCube::Move::R_PRIME: return "R'";
            case GenericRubiksCube::Move::R2:      return "R2";
            case GenericRubiksCube::Move::U:       return "U";
            case GenericRubiksCube::Move::U_PRIME: return "U'";
            case GenericRubiksCube::Move::U2:      return "U2";
            case GenericRubiksCube::Move::D:       return "D";
            case GenericRubiksCube::Move::D_PRIME: return "D'";
            case GenericRubiksCube::Move::D2:      return "D2";
            case GenericRubiksCube::Move::F:       return "F";
            case GenericRubiksCube::Move::F_PRIME: return "F'";
            case GenericRubiksCube::Move::F2:      return "F2";
            case GenericRubiksCube::Move::B:       return "B";
            case GenericRubiksCube::Move::B_PRIME: return "B'";
            case GenericRubiksCube::Move::B2:      return "B2";
            default:                               return "??";
        }
    }

} // namespace MoveUtils

#endif // MOVE_UTILS_H
