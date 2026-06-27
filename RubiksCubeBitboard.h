/**
 * @file RubiksCubeBitboard.h
 * @brief High-performance Rubik's Cube model packed into three 64-bit integers.
 *
 * @par Bit layout
 * The 54 stickers are packed 3 bits each into three @c uint64_t registers:
 * @code
 *   board[0]: stickers  0–17  (bits  0–53, 54 bits used of 64)
 *   board[1]: stickers 18–35  (bits  0–53, 54 bits used of 64)
 *   board[2]: stickers 36–53  (bits  0–53, 54 bits used of 64)
 * @endcode
 *
 * Within each register, sticker @c k (relative to the register's base) occupies:
 * @code
 *   bits [(k % 18) * 3 + 2 : (k % 18) * 3]
 * @endcode
 *
 * Colour encoding (3 bits, values 0–5):
 * @code
 *   0 = Up    (White)    3 = Right (Blue)
 *   1 = Left  (Green)    4 = Back  (Orange)
 *   2 = Front (Red)      5 = Down  (Yellow)
 * @endcode
 *
 * @par Why three registers?
 * Each register holds exactly 18 stickers × 3 bits = 54 bits. Three registers
 * provide 192 bits total, of which 162 are used. The layout is chosen so that
 * a move's cycle() calls never straddle a register boundary mid-sticker — each
 * sticker's 3 bits always fall within a single register.
 *
 * @par Performance characteristics
 *   - getSticker / setSticker: 2–3 instructions (shift + mask).
 *   - cycle():                 12 bit operations.
 *   - Entire cube state:       3 × 64-bit integers (fits in CPU registers).
 *   - isSolved():              54 comparisons on packed integers.
 *
 * This model is designed for the IDA* inner loop where node expansion speed
 * dominates total solve time.
 */

#ifndef RUBIKS_CUBE_BITBOARD_H
#define RUBIKS_CUBE_BITBOARD_H

#include "GenericRubiksCube.h"
#include <cstdint>
#include <cassert>
#include <string>

/**
 * @class RubiksCubeBitboard
 * @brief Concrete Rubik's Cube backed by three packed uint64_t registers.
 * @extends GenericRubiksCube
 */
class RubiksCubeBitboard : public GenericRubiksCube {
private:

    /**
     * @brief The three 64-bit registers holding the full 162-bit cube state.
     *
     * @see RubiksCubeBitboard.h for the complete bit-layout specification.
     */
    uint64_t board[3];

    // -----------------------------------------------------------------------
    // Private bitwise helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Reads the 3-bit colour code stored at sticker @p index.
     *
     * Determines the register and bit offset from @p index, then extracts the
     * 3-bit value using a right-shift and mask.
     *
     * Bit position: register = index / 18, shift = (index % 18) * 3.
     * The 3 target bits sit at positions [shift+2 : shift] of board[register].
     *
     * @param index Sticker index in [0, 53].
     * @return The 3-bit colour code (0–5).
     */
    uint8_t getSticker(int index) const;

    /**
     * @brief Writes a 3-bit colour code to sticker @p index.
     *
     * Clears the three target bits with an inverted mask, then ORs in the new
     * value. Both the clear and set operations are confined to 3 bits so that
     * adjacent stickers are never affected.
     *
     * @param index Sticker index in [0, 53].
     * @param color Colour code in [0, 5]. Values above 5 are undefined behaviour.
     */
    void setSticker(int index, uint8_t color);

    /**
     * @brief Performs a 4-way cyclic permutation of four sticker positions.
     *
     * Shifts colours around the cycle: i4 ← i3 ← i2 ← i1 ← (old i4).
     * This is the atomic operation used to implement all 18 face rotations.
     *
     * @param i1 First sticker index in the cycle.
     * @param i2 Second sticker index.
     * @param i3 Third sticker index.
     * @param i4 Fourth sticker index.
     */
    void cycle(int i1, int i2, int i3, int i4);

public:

    /**
     * @brief Constructs a fully solved Rubik's Cube.
     *
     * Zeroes all three registers, then writes each sticker's home face index
     * (0–5) into its packed position.
     */
    RubiksCubeBitboard();

    // -----------------------------------------------------------------------
    // GenericRubiksCube interface
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if every sticker matches its home face index.
     *
     * In a solved state, sticker @c i always holds colour @c (i / 9).
     * This function iterates all 54 stickers and verifies this invariant.
     *
     * @return @c true when solved, @c false when scrambled.
     */
    bool isSolved() const override;

    /**
     * @brief Applies a move to the bitboard and returns @c *this for chaining.
     *
     * All 18 moves are implemented as sequences of cycle() calls on the packed
     * sticker indices, identical in structure to the 1D-array model but operating
     * on 3-bit packed values instead of chars.
     *
     * @param move One of the 18 standard SiGN moves.
     * @return Reference to this cube.
     */
    GenericRubiksCube& performMove(Move move) override;

    /**
     * @brief Prints the raw register values to standard output.
     *
     * Output format: @c "Bitboard Engine Data: [board[0], board[1], board[2]]"
     * Not a human-readable cube layout — use RubiksCube3dArray::print() for that.
     */
    void print() const override;

    /**
     * @brief Extracts all 8 corner states as a 24-character numeric string.
     *
     * Each sticker's colour is encoded as a single digit character '0'–'5'
     * (using @c to_string on the 3-bit colour code). Corner order and sticker
     * order within each corner match the GenericRubiksCube::getCorners() convention.
     *
     * @note The output uses digit characters, not letter characters ('W', 'G'…).
     *       This is intentional — the Corner Pattern Database was generated from
     *       this model, so the key format must match.
     *
     * @return 24-character string encoding all corner sticker colour codes.
     */
    std::string getCorners() const override;

    /**
     * @brief Serialises the full state as an underscore-delimited register string.
     *
     * Format: @c "board[0]_board[1]_board[2]" in decimal.
     * Used by BFS to detect visited states when the Bitboard model is the
     * backing representation.
     *
     * @return Unique state string for the current cube configuration.
     */
    std::string getStateString() const override;
};

#endif // RUBIKS_CUBE_BITBOARD_H
