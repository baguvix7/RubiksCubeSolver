/**
 * @file RubiksCube3dArray.h
 * @brief Rubik's Cube model backed by a 3D character array.
 *
 * This is the most human-readable of the three concrete representations.
 * The cube state is stored as @c cube[face][row][col] where:
 *   - face ∈ {0=Up, 1=Left, 2=Front, 3=Right, 4=Back, 5=Down}
 *   - row  ∈ {0=top, 1=middle, 2=bottom}
 *   - col  ∈ {0=left, 1=centre, 2=right}
 *
 * Each sticker stores one ASCII colour character:
 *   'W'=White(Up), 'G'=Green(Left), 'R'=Red(Front),
 *   'B'=Blue(Right), 'O'=Orange(Back), 'Y'=Yellow(Down)
 *
 * @par When to use this model
 *   - Verifying that rotation math is correct (print() is the clearest).
 *   - BFS / IDDFS / DFS solvers that operate on shallow scrambles.
 *   - Unit-testing new move sequences before porting them to faster models.
 *
 * @par Performance note
 *   The 3D layout has poor spatial locality — adjacent stickers on the same
 *   face are not contiguous in memory. For performance-critical solvers
 *   (IDA* with millions of node expansions), prefer RubiksCubeBitboard.
 */

#ifndef RUBIKS_CUBE_3D_ARRAY_H
#define RUBIKS_CUBE_3D_ARRAY_H

#include "GenericRubiksCube.h"

/**
 * @class RubiksCube3dArray
 * @brief Concrete Rubik's Cube using a [6][3][3] character array.
 * @extends GenericRubiksCube
 */
class RubiksCube3dArray : public GenericRubiksCube {
private:

    /**
     * @brief The backing store for all 54 sticker colours.
     *
     * Indexing: @c cube[face][row][col]
     *   face: 0=Up(W)  1=Left(G)  2=Front(R)  3=Right(B)  4=Back(O)  5=Down(Y)
     *   row : 0=top    1=middle   2=bottom
     *   col : 0=left   1=centre   2=right
     */
    char cube[6][3][3];

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Rotates all nine stickers on a single face 90° clockwise.
     *
     * This is a pure in-place transpose-and-reverse operation on @c cube[face].
     * It does NOT move the adjacent edge stickers — that is the responsibility
     * of each individual face method (u(), d(), etc.).
     *
     * @param face Index of the face to rotate (0–5).
     */
    void rotateFaceClockwise(int face);

    /**
     * @name Single-face clockwise move implementations
     * @{
     * Each function performs one 90° clockwise turn of its named face, including
     * cycling the three adjacent edge strips into their new positions.
     * Prime and half-turn moves are derived by calling the clockwise variant
     * 3 or 2 times respectively inside performMove().
     */
    void u(); ///< Up face — clockwise 90°
    void d(); ///< Down face — clockwise 90°
    void l(); ///< Left face — clockwise 90°
    void r(); ///< Right face — clockwise 90°
    void f(); ///< Front face — clockwise 90°
    void b(); ///< Back face — clockwise 90°
    /** @} */

public:

    /**
     * @brief Constructs a solved Rubik's Cube.
     *
     * Initialises every sticker to the colour of its home face:
     * Up=W, Left=G, Front=R, Right=B, Back=O, Down=Y.
     */
    RubiksCube3dArray();

    // -----------------------------------------------------------------------
    // GenericRubiksCube interface
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if every sticker matches its face's centre colour.
     * @return @c true when solved, @c false when scrambled.
     */
    bool isSolved() const override;

    /**
     * @brief Applies a move to the cube and returns @c *this for chaining.
     * @param move One of the 18 standard SiGN moves.
     * @return Reference to this cube.
     */
    GenericRubiksCube& performMove(Move move) override;

    /**
     * @brief Prints an unrolled cross-layout to standard output.
     *
     * Format (offsets reflect the standard cube net):
     * @code
     *       UUU
     *       UUU
     *       UUU
     *  GGG  RRR  BBB  OOO
     *  GGG  RRR  BBB  OOO
     *  GGG  RRR  BBB  OOO
     *       YYY
     *       YYY
     *       YYY
     * @endcode
     */
    void print() const override;

    /**
     * @brief Extracts the 8 corner states as a 24-character string.
     *
     * Corner order and sticker order within each corner match the convention
     * used by CornerDBGenerator. See GenericRubiksCube::getCorners() for the
     * canonical corner ordering.
     *
     * @return 24-character string encoding all corner sticker colours.
     */
    std::string getCorners() const override;

    /**
     * @brief Serialises the full 54-sticker state to a 54-character string.
     *
     * Iterates faces in order 0–5, rows 0–2, columns 0–2.
     * Used by BFSSolver to detect and skip already-visited cube states.
     *
     * @return 54-character state string.
     */
    std::string getStateString() const override;
};

#endif // RUBIKS_CUBE_3D_ARRAY_H
