/**
 * @file RubiksCube1dArray.h
 * @brief Rubik's Cube model backed by a contiguous 1D character array.
 *
 * This representation flattens the conceptual [6][3][3] layout into a single
 * 54-byte array. The index for any sticker is:
 * @code
 *   index = (face * 9) + (row * 3) + col
 * @endcode
 *
 * Compared to RubiksCube3dArray:
 *   - All 54 stickers sit in a single contiguous block → better cache locality.
 *   - The 4-way cyclic @c cycle() helper simplifies move code.
 *   - Slightly harder to read than [face][row][col] indexing.
 *
 * Compared to RubiksCubeBitboard:
 *   - Easier to debug (plain chars vs bit-packed integers).
 *   - Larger memory footprint per node (54 bytes vs ~21 bytes).
 *   - Slower sticker access (no bit operations needed, but no register packing).
 *
 * @par Face layout
 * @code
 *   0–8  : Up    (White  'W')
 *   9–17 : Left  (Green  'G')
 *   18–26: Front (Red    'R')
 *   27–35: Right (Blue   'B')
 *   36–44: Back  (Orange 'O')
 *   45–53: Down  (Yellow 'Y')
 * @endcode
 */

#ifndef RUBIKS_CUBE_1D_ARRAY_H
#define RUBIKS_CUBE_1D_ARRAY_H

#include "GenericRubiksCube.h"
#include <string>

/**
 * @class RubiksCube1dArray
 * @brief Concrete Rubik's Cube using a flat 54-element character array.
 * @extends GenericRubiksCube
 */
class RubiksCube1dArray : public GenericRubiksCube {
private:

    /**
     * @brief The entire cube state as a single 54-byte contiguous array.
     *
     * Sticker index formula: @c cube[(face * 9) + (row * 3) + col]
     *   face: 0=Up  1=Left  2=Front  3=Right  4=Back  5=Down
     *   row : 0=top 1=mid   2=bottom
     *   col : 0=left 1=centre 2=right
     */
    char cube[54];

    /**
     * @brief Performs a 4-way cyclic swap of four sticker positions.
     *
     * Rotates colours around the cycle: i4 ← i3 ← i2 ← i1 ← i4.
     * Used as the atomic building block for every move implementation.
     *
     * @param i1 Index of the first sticker in the cycle.
     * @param i2 Index of the second sticker.
     * @param i3 Index of the third sticker.
     * @param i4 Index of the fourth sticker.
     */
    void cycle(int i1, int i2, int i3, int i4);

public:

    /**
     * @brief Constructs a solved Rubik's Cube.
     *
     * Fills each face's 9 stickers with its home colour character.
     */
    RubiksCube1dArray();

    // -----------------------------------------------------------------------
    // GenericRubiksCube interface
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if every face is a uniform colour.
     * @return @c true when solved, @c false when scrambled.
     */
    bool isSolved() const override;

    /**
     * @brief Applies a move and returns @c *this for chaining.
     * @param move One of the 18 standard SiGN moves.
     * @return Reference to this cube.
     */
    GenericRubiksCube& performMove(Move move) override;

    /**
     * @brief Prints an unrolled cross-layout to standard output.
     */
    void print() const override;

    /**
     * @brief Extracts all 8 corner states as a 24-character string.
     *
     * Sticker indices are computed from the 1D formula. Corner order matches
     * the convention in GenericRubiksCube::getCorners().
     *
     * @return 24-character corner state string.
     */
    std::string getCorners() const override;

    /**
     * @brief Returns the full 54-character state string.
     *
     * A simple concatenation of all elements in @c cube[0..53].
     * Used by BFS to track visited states.
     *
     * @return 54-character state string.
     */
    std::string getStateString() const override;
};

#endif // RUBIKS_CUBE_1D_ARRAY_H
