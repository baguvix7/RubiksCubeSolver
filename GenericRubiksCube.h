/**
 * @file GenericRubiksCube.h
 * @brief Abstract base class defining the standard interface for all Rubik's Cube models.
 *
 * This file forms the backbone of the solver's polymorphic design. By coding
 * all graph-search algorithms (BFS, DFS, IDDFS, IDA*) against this interface
 * rather than against concrete types, we achieve complete decoupling between
 * the *search strategy* and the *data representation*.
 *
 * Three concrete implementations exist:
 *   - RubiksCube3dArray  — readable 3D char array; best for debugging rotations.
 *   - RubiksCube1dArray  — flattened 1D array; better cache locality than 3D.
 *   - RubiksCubeBitboard — entire state packed into three uint64_t registers;
 *                          fastest for the IDA* hot path.
 *
 * Adding a fourth representation (e.g. a SIMD-accelerated model) requires only
 * implementing this interface — no solver code needs to change.
 *
 * @note Move enum ordering is load-bearing. The inverse-move formula in
 *       MoveUtils::inverse() and the redundancy pruner in MoveUtils::isRedundant()
 *       both depend on moves being grouped in contiguous triplets per face.
 *       Do not reorder the enum values.
 */

#ifndef GENERIC_RUBIKS_CUBE_H
#define GENERIC_RUBIKS_CUBE_H

#include <iostream>
#include <string>

/**
 * @class GenericRubiksCube
 * @brief Pure-virtual interface for all Rubik's Cube data model implementations.
 *
 * Concrete subclasses must implement all five pure-virtual methods.
 * The virtual destructor ensures correct polymorphic cleanup through base pointers.
 */
class GenericRubiksCube {
public:

    // =========================================================================
    // Move Enumeration
    // =========================================================================

    /**
     * @enum Move
     * @brief The 18 standard moves in Singmaster (SiGN) notation.
     *
     * Moves are grouped in strict triplets, one triplet per face:
     * @code
     *   Face  | Clockwise (CW) | Counter-CW (Prime) | Half-turn (2)
     *   ------+----------------+--------------------+--------------
     *   Left  | L  (0)         | L' (1)             | L2 (2)
     *   Right | R  (3)         | R' (4)             | R2 (5)
     *   Up    | U  (6)         | U' (7)             | U2 (8)
     *   Down  | D  (9)         | D' (10)            | D2 (11)
     *   Front | F  (12)        | F' (13)            | F2 (14)
     *   Back  | B  (15)        | B' (16)            | B2 (17)
     * @endcode
     *
     * This grouping is exploited by MoveUtils::inverse() and
     * MoveUtils::isRedundant(). Within each triplet:
     *   - @c enum_value % 3 == 0  → Clockwise 90°
     *   - @c enum_value % 3 == 1  → Counter-clockwise 90° (Prime)
     *   - @c enum_value % 3 == 2  → Half-turn 180°
     *
     * @warning Reordering these values will silently break move inversion and
     *          same-face redundancy pruning in all solvers.
     */
    enum class Move {
        L,  L_PRIME,  L2,   ///< Left face:  CW, CCW, 180°
        R,  R_PRIME,  R2,   ///< Right face: CW, CCW, 180°
        U,  U_PRIME,  U2,   ///< Up face:    CW, CCW, 180°
        D,  D_PRIME,  D2,   ///< Down face:  CW, CCW, 180°
        F,  F_PRIME,  F2,   ///< Front face: CW, CCW, 180°
        B,  B_PRIME,  B2    ///< Back face:  CW, CCW, 180°
    };

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Virtual destructor.
     *
     * Required to ensure correct destruction of derived objects when deleted
     * through a GenericRubiksCube pointer or reference.
     */
    virtual ~GenericRubiksCube() = default;

    // =========================================================================
    // Core Interface — all subclasses must implement these
    // =========================================================================

    /**
     * @brief Checks whether the cube is in a perfectly solved state.
     *
     * A solved cube has all nine stickers on each face matching that face's
     * centre colour. The centre sticker never moves relative to the core, so
     * it acts as the reference colour for its face.
     *
     * @return @c true if every sticker matches its face centre; @c false otherwise.
     */
    virtual bool isSolved() const = 0;

    /**
     * @brief Applies a single move to the cube, mutating it in place.
     *
     * Returns a reference to @c *this to support method chaining:
     * @code
     *   cube.performMove(Move::U).performMove(Move::R).performMove(Move::F);
     * @endcode
     *
     * @param move The SiGN-notation move to apply.
     * @return A reference to this object (for chaining).
     */
    virtual GenericRubiksCube& performMove(Move move) = 0;

    /**
     * @brief Prints an unrolled cross-layout of the cube to standard output.
     *
     * Intended for debugging. The exact format depends on the concrete
     * implementation but should show all six faces in a human-readable layout.
     */
    virtual void print() const = 0;

    /**
     * @brief Serialises the state of all eight corners into a compact string.
     *
     * Each corner contributes three sticker colours (one per touching face),
     * giving a 24-character string for 8 corners. This string is used as the
     * lookup key into the Corner Pattern Database during IDA* heuristic queries.
     *
     * Corner order must be consistent across all implementations so that a
     * database generated from one model can be queried by another:
     * @code
     *   0: UBL  1: UBR  2: UFL  3: UFR
     *   4: DBL  5: DBR  6: DFL  7: DFR
     * @endcode
     *
     * @return A 24-character string uniquely representing the corner configuration.
     */
    virtual std::string getCorners() const = 0;

    /**
     * @brief Serialises the entire cube state into a flat string for hashing.
     *
     * Used by BFS to track visited states. The string must be:
     *   - Deterministic: same cube state always produces the same string.
     *   - Injective: different cube states always produce different strings.
     *
     * @return A string uniquely representing the complete 54-sticker state.
     */
    virtual std::string getStateString() const = 0;
};

#endif // GENERIC_RUBIKS_CUBE_H
