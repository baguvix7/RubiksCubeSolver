/**
 * @file RubiksCube1dArray.cpp
 * @brief Move logic and serialisation for the 1D-array Rubik's Cube model.
 *
 * Every move is implemented as:
 *   1. Two cycle() calls to rotate the face's own corner and edge stickers.
 *   2. Three cycle() calls to shift the three affected adjacent-edge strips.
 *
 * The index constants below were derived from the sticker layout:
 *   index = (face * 9) + (row * 3) + col
 *
 * For prime (CCW) and half-turn moves, we simply chain the clockwise
 * primitive 3 or 2 times respectively — this avoids writing 18 separate
 * implementations at the cost of 2–6 extra cycle() calls for prime/half moves.
 */

#include "RubiksCube1dArray.h"
#include <iostream>

using namespace std;

// =========================================================================
// Constructor
// =========================================================================

RubiksCube1dArray::RubiksCube1dArray() {
    // Sticker ranges per face (each face occupies 9 consecutive indices):
    //   Up   [0 – 8]  : 'W'
    //   Left [9 – 17] : 'G'
    //   Front[18–26]  : 'R'
    //   Right[27–35]  : 'B'
    //   Back [36–44]  : 'O'
    //   Down [45–53]  : 'Y'
    for (int i =  0; i <  9; i++) cube[i] = 'W';
    for (int i =  9; i < 18; i++) cube[i] = 'G';
    for (int i = 18; i < 27; i++) cube[i] = 'R';
    for (int i = 27; i < 36; i++) cube[i] = 'B';
    for (int i = 36; i < 45; i++) cube[i] = 'O';
    for (int i = 45; i < 54; i++) cube[i] = 'Y';
}

// =========================================================================
// isSolved
// =========================================================================

bool RubiksCube1dArray::isSolved() const {
    for (int i =  0; i <  9; i++) if (cube[i] != 'W') return false;
    for (int i =  9; i < 18; i++) if (cube[i] != 'G') return false;
    for (int i = 18; i < 27; i++) if (cube[i] != 'R') return false;
    for (int i = 27; i < 36; i++) if (cube[i] != 'B') return false;
    for (int i = 36; i < 45; i++) if (cube[i] != 'O') return false;
    for (int i = 45; i < 54; i++) if (cube[i] != 'Y') return false;
    return true;
}

// =========================================================================
// print
// =========================================================================

void RubiksCube1dArray::print() const {
    cout << "Rubik's Cube (1D Array Model)\n\n";

    // Up face (rows 0–2 of the [0–8] block)
    for (int row = 0; row < 3; row++) {
        cout << "      ";
        for (int col = 0; col < 3; col++) cout << cube[row * 3 + col] << " ";
        cout << "\n";
    }
    cout << "\n";

    // Middle band: Left | Front | Right | Back (each face's rows 0–2)
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) cout << cube[ 9 + row * 3 + col] << " "; // Left
        cout << " ";
        for (int col = 0; col < 3; col++) cout << cube[18 + row * 3 + col] << " "; // Front
        cout << " ";
        for (int col = 0; col < 3; col++) cout << cube[27 + row * 3 + col] << " "; // Right
        cout << " ";
        for (int col = 0; col < 3; col++) cout << cube[36 + row * 3 + col] << " "; // Back
        cout << "\n";
    }
    cout << "\n";

    // Down face (rows 0–2 of the [45–53] block)
    for (int row = 0; row < 3; row++) {
        cout << "      ";
        for (int col = 0; col < 3; col++) cout << cube[45 + row * 3 + col] << " ";
        cout << "\n";
    }
    cout << "\n";
}

// =========================================================================
// cycle — 4-way cyclic swap
// =========================================================================

void RubiksCube1dArray::cycle(int i1, int i2, int i3, int i4) {
    // Rotates: i4 ← i3 ← i2 ← i1 ← (old i4)
    const char temp = cube[i4];
    cube[i4] = cube[i3];
    cube[i3] = cube[i2];
    cube[i2] = cube[i1];
    cube[i1] = temp;
}

// =========================================================================
// performMove
// =========================================================================

GenericRubiksCube& RubiksCube1dArray::performMove(Move move) {
    switch (move) {
        // ---- UP (indices 0–8) ----
        case Move::U:
            cycle(0, 2, 8, 6); // Face corners CW
            cycle(1, 5, 7, 3); // Face edges  CW
            // Adjacent top rows: Back[36-38] → Left[9-11] → Front[18-20] → Right[27-29]
            cycle(36, 27, 18, 9);
            cycle(37, 28, 19, 10);
            cycle(38, 29, 20, 11);
            break;
        case Move::U_PRIME: performMove(Move::U); performMove(Move::U); performMove(Move::U); break;
        case Move::U2:      performMove(Move::U); performMove(Move::U); break;

        // ---- LEFT (indices 9–17) ----
        case Move::L:
            cycle(9, 11, 17, 15); // Face corners CW
            cycle(10, 14, 16, 12); // Face edges  CW
            // Adjacent left columns: Up[0,3,6] → Front[18,21,24] → Down[45,48,51] → Back[44,41,38]
            cycle(0, 18, 45, 44);
            cycle(3, 21, 48, 41);
            cycle(6, 24, 51, 38);
            break;
        case Move::L_PRIME: performMove(Move::L); performMove(Move::L); performMove(Move::L); break;
        case Move::L2:      performMove(Move::L); performMove(Move::L); break;

        // ---- FRONT (indices 18–26) ----
        case Move::F:
            cycle(18, 20, 26, 24); // Face corners CW
            cycle(19, 23, 25, 21); // Face edges  CW
            // Adjacent strips: Up-bottom[6,7,8] → Right-left[27,30,33] → Down-top[47,46,45] → Left-right[17,14,11]
            cycle(6, 27, 47, 17);
            cycle(7, 30, 46, 14);
            cycle(8, 33, 45, 11);
            break;
        case Move::F_PRIME: performMove(Move::F); performMove(Move::F); performMove(Move::F); break;
        case Move::F2:      performMove(Move::F); performMove(Move::F); break;

        // ---- RIGHT (indices 27–35) ----
        case Move::R:
            cycle(27, 29, 35, 33); // Face corners CW
            cycle(28, 32, 34, 30); // Face edges  CW
            // Adjacent right columns: Up[8,5,2] → Back[36,39,42] → Down[53,50,47] → Front[26,23,20]
            cycle(8, 36, 53, 26);
            cycle(5, 39, 50, 23);
            cycle(2, 42, 47, 20);
            break;
        case Move::R_PRIME: performMove(Move::R); performMove(Move::R); performMove(Move::R); break;
        case Move::R2:      performMove(Move::R); performMove(Move::R); break;

        // ---- BACK (indices 36–44) ----
        case Move::B:
            cycle(36, 38, 44, 42); // Face corners CW
            cycle(37, 41, 43, 39); // Face edges  CW
            // Adjacent strips: Up-top[2,1,0] → Left-left[9,12,15] → Down-bottom[51,52,53] → Right-right[35,32,29]
            cycle(2, 9, 51, 35);
            cycle(1, 12, 52, 32);
            cycle(0, 15, 53, 29);
            break;
        case Move::B_PRIME: performMove(Move::B); performMove(Move::B); performMove(Move::B); break;
        case Move::B2:      performMove(Move::B); performMove(Move::B); break;

        // ---- DOWN (indices 45–53) ----
        case Move::D:
            cycle(45, 47, 53, 51); // Face corners CW
            cycle(46, 50, 52, 48); // Face edges  CW
            // Adjacent bottom rows: Front[24,25,26] → Right[33,34,35] → Back[42,43,44] → Left[15,16,17]
            cycle(24, 33, 42, 15);
            cycle(25, 34, 43, 16);
            cycle(26, 35, 44, 17);
            break;
        case Move::D_PRIME: performMove(Move::D); performMove(Move::D); performMove(Move::D); break;
        case Move::D2:      performMove(Move::D); performMove(Move::D); break;
    }
    return *this;
}

// =========================================================================
// getCorners
// =========================================================================

string RubiksCube1dArray::getCorners() const {
    string corners;
    corners.reserve(24); // 8 corners × 3 stickers

    // Index derivation: (face*9) + (row*3) + col
    // Canonical order: UBL, UBR, UFL, UFR, DBL, DBR, DFL, DFR

    corners += cube[0];  corners += cube[38]; corners += cube[9];  // UBL: Up[0], Back[2], Left[0]
    corners += cube[2];  corners += cube[36]; corners += cube[29]; // UBR: Up[2], Back[0], Right[2]
    corners += cube[6];  corners += cube[18]; corners += cube[11]; // UFL: Up[6], Front[0], Left[2]
    corners += cube[8];  corners += cube[20]; corners += cube[27]; // UFR: Up[8], Front[2], Right[0]
    corners += cube[51]; corners += cube[44]; corners += cube[15]; // DBL: Down[6], Back[8], Left[6]
    corners += cube[53]; corners += cube[42]; corners += cube[35]; // DBR: Down[8], Back[6], Right[8]
    corners += cube[45]; corners += cube[24]; corners += cube[17]; // DFL: Down[0], Front[6], Left[8]
    corners += cube[47]; corners += cube[26]; corners += cube[33]; // DFR: Down[2], Front[8], Right[6]

    return corners;
}

// =========================================================================
// getStateString
// =========================================================================

string RubiksCube1dArray::getStateString() const {
    string state;
    state.reserve(54); // Avoid repeated reallocation in the hot path
    for (int i = 0; i < 54; i++) {
        state += cube[i];
    }
    return state;
}
