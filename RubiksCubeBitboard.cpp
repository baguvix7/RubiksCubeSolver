/**
 * @file RubiksCubeBitboard.cpp
 * @brief Bitwise operations and full 18-move logic for the Bitboard cube model.
 *
 * All cycle() index arguments correspond to the same sticker numbering used by
 * RubiksCube1dArray (index = face*9 + row*3 + col), making it straightforward
 * to verify correctness by comparing both models on the same scramble sequence.
 *
 * The B-face move (Back) is the only one that differs non-trivially from an
 * intuitive reading; the adjacent index order must account for the fact that
 * the Back face is viewed from behind, reversing the apparent row/column order.
 */

#include "RubiksCubeBitboard.h"
#include <iostream>
#include <string>
#include <cassert>

using namespace std;

// =========================================================================
// Constructor
// =========================================================================

RubiksCubeBitboard::RubiksCubeBitboard() {
    // Zero all three registers first
    board[0] = board[1] = board[2] = 0ULL;

    // Write each sticker's home face index: sticker i belongs to face (i / 9)
    for (int i = 0; i < 54; i++) {
        setSticker(i, static_cast<uint8_t>(i / 9));
    }
}

// =========================================================================
// Bitwise helpers
// =========================================================================

uint8_t RubiksCubeBitboard::getSticker(int index) const {
    assert(index >= 0 && index < 54 && "getSticker: index out of [0, 53]");

    const int reg   = index / 18;
    const int shift = (index % 18) * 3;

    return static_cast<uint8_t>((board[reg] >> shift) & 0x7ULL);
}

void RubiksCubeBitboard::setSticker(int index, uint8_t color) {
    assert(index >= 0 && index < 54 && "setSticker: index out of [0, 53]");
    assert(color <= 5 && "setSticker: color exceeds valid face range [0, 5]");

    const int reg   = index / 18;
    const int shift = (index % 18) * 3;

    // Clear the 3 bits at [shift+2 : shift], then write the new color value
    board[reg] &= ~(0x7ULL << shift);
    board[reg] |=  (static_cast<uint64_t>(color & 0x7) << shift);
}

void RubiksCubeBitboard::cycle(int i1, int i2, int i3, int i4) {
    // Rotates: i4 ← i3 ← i2 ← i1 ← (old i4)
    const uint8_t temp = getSticker(i4);
    setSticker(i4, getSticker(i3));
    setSticker(i3, getSticker(i2));
    setSticker(i2, getSticker(i1));
    setSticker(i1, temp);
}

// =========================================================================
// performMove — full 18-move implementation
// =========================================================================

GenericRubiksCube& RubiksCubeBitboard::performMove(Move move) {
    switch (move) {

        // ------------------------------------
        // UP FACE (stickers 0–8)
        // ------------------------------------
        case Move::U:
            cycle(0, 2, 8, 6);       // Face corners CW
            cycle(1, 5, 7, 3);       // Face edges   CW
            // Adjacent top rows: Front[18-20] → Left[9-11] → Back[36-38] → Right[27-29]
            cycle(18, 9, 36, 27);
            cycle(19, 10, 37, 28);
            cycle(20, 11, 38, 29);
            break;
        case Move::U_PRIME:
            performMove(Move::U); performMove(Move::U); performMove(Move::U); break;
        case Move::U2:
            performMove(Move::U); performMove(Move::U); break;

        // ------------------------------------
        // DOWN FACE (stickers 45–53)
        // ------------------------------------
        case Move::D:
            cycle(45, 47, 53, 51);   // Face corners CW
            cycle(46, 50, 52, 48);   // Face edges   CW
            // Adjacent bottom rows: Front[24-26] → Right[33-35] → Back[42-44] → Left[15-17]
            cycle(24, 33, 42, 15);
            cycle(25, 34, 43, 16);
            cycle(26, 35, 44, 17);
            break;
        case Move::D_PRIME:
            performMove(Move::D); performMove(Move::D); performMove(Move::D); break;
        case Move::D2:
            performMove(Move::D); performMove(Move::D); break;

        // ------------------------------------
        // LEFT FACE (stickers 9–17)
        // ------------------------------------
        case Move::L:
            cycle(9, 11, 17, 15);    // Face corners CW
            cycle(10, 14, 16, 12);   // Face edges   CW
            // Adjacent left columns: Up[0,3,6] → Front[18,21,24] → Down[45,48,51] → Back[44,41,38]
            cycle(0, 18, 45, 44);
            cycle(3, 21, 48, 41);
            cycle(6, 24, 51, 38);
            break;
        case Move::L_PRIME:
            performMove(Move::L); performMove(Move::L); performMove(Move::L); break;
        case Move::L2:
            performMove(Move::L); performMove(Move::L); break;

        // ------------------------------------
        // RIGHT FACE (stickers 27–35)
        // ------------------------------------
        case Move::R:
            cycle(27, 29, 35, 33);   // Face corners CW
            cycle(28, 32, 34, 30);   // Face edges   CW
            // Adjacent right columns: Up[8,5,2] → Back[36,39,42] → Down[53,50,47] → Front[26,23,20]
            cycle(8, 36, 53, 26);
            cycle(5, 39, 50, 23);
            cycle(2, 42, 47, 20);
            break;
        case Move::R_PRIME:
            performMove(Move::R); performMove(Move::R); performMove(Move::R); break;
        case Move::R2:
            performMove(Move::R); performMove(Move::R); break;

        // ------------------------------------
        // FRONT FACE (stickers 18–26)
        // ------------------------------------
        case Move::F:
            cycle(18, 20, 26, 24);   // Face corners CW
            cycle(19, 23, 25, 21);   // Face edges   CW
            // Adjacent strips: Up-bottom[6,7,8] → Right-left[27,30,33] → Down-top[47,46,45] → Left-right[17,14,11]
            cycle(6, 27, 47, 17);
            cycle(7, 30, 46, 14);
            cycle(8, 33, 45, 11);
            break;
        case Move::F_PRIME:
            performMove(Move::F); performMove(Move::F); performMove(Move::F); break;
        case Move::F2:
            performMove(Move::F); performMove(Move::F); break;

        // ------------------------------------
        // BACK FACE (stickers 36–44)
        // ------------------------------------
        case Move::B:
            cycle(36, 38, 44, 42);   // Face corners CW
            cycle(37, 41, 43, 39);   // Face edges   CW
            // Adjacent strips: Up-top[2,1,0] → Left-left[9,12,15] → Down-bottom[51,52,53] → Right-right[35,32,29]
            cycle(2, 9, 45, 35);
            cycle(1, 12, 46, 32);
            cycle(0, 15, 47, 29);
            break;
        case Move::B_PRIME:
            performMove(Move::B); performMove(Move::B); performMove(Move::B); break;
        case Move::B2:
            performMove(Move::B); performMove(Move::B); break;
    }
    return *this;
}

// =========================================================================
// isSolved
// =========================================================================

bool RubiksCubeBitboard::isSolved() const {
    // In a solved state, sticker i always holds colour (i / 9).
    for (int i = 0; i < 54; i++) {
        if (getSticker(i) != static_cast<uint8_t>(i / 9)) return false;
    }
    return true;
}

// =========================================================================
// getStateString
// =========================================================================

string RubiksCubeBitboard::getStateString() const {
    // Encodes all three registers as a delimited decimal string.
    // Different cube states always produce different strings (injective) because
    // the three registers together hold the full 162-bit state with no collisions.
    return to_string(board[0]) + "_" + to_string(board[1]) + "_" + to_string(board[2]);
}

// =========================================================================
// getCorners
// =========================================================================

string RubiksCubeBitboard::getCorners() const {
    // The 24 sticker indices covering all 8 corners (3 stickers each).
    // Order: UBL, UBR, UFL, UFR, DBL, DBR, DFL, DFR
    // Each triplet lists: (U/D sticker, adjacent-face sticker, adjacent-face sticker)
    static const int CORNER_INDICES[24] = {
         0, 38,  9,   // UBL: Up[0],    Back[2],  Left[0]
         2, 36, 29,   // UBR: Up[2],    Back[0],  Right[2]
         6, 18, 11,   // UFL: Up[6],    Front[0], Left[2]
         8, 20, 27,   // UFR: Up[8],    Front[2], Right[0]
        51, 44, 15,   // DBL: Down[6],  Back[8],  Left[6]
        53, 42, 35,   // DBR: Down[8],  Back[6],  Right[8]
        45, 24, 17,   // DFL: Down[0],  Front[6], Left[8]
        47, 26, 33    // DFR: Down[2],  Front[8], Right[6]
    };

    string c;
    c.reserve(24);
    for (int i = 0; i < 24; i++) {
        // Colour codes are 0–5; convert to char digit '0'–'5'
        c += static_cast<char>('0' + getSticker(CORNER_INDICES[i]));
    }
    return c;
}

// =========================================================================
// print
// =========================================================================

void RubiksCubeBitboard::print() const {
    // Raw register dump — not a human-readable layout.
    // Use RubiksCube3dArray::print() for visual verification.
    cout << "Bitboard registers: ["
         << board[0] << ", "
         << board[1] << ", "
         << board[2] << "]\n";
}
