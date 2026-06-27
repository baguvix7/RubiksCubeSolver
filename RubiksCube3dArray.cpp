/**
 * @file RubiksCube3dArray.cpp
 * @brief Rotation logic and state serialisation for the 3D-array cube model.
 *
 * All rotation functions follow the same pattern:
 *   1. Call rotateFaceClockwise() to spin the face's own nine stickers.
 *   2. Shift the three adjacent edge strips using a temporary buffer.
 *
 * The edge-shift direction for each face was verified empirically against
 * a physical cube. If you suspect a rotation is wrong, call print() before
 * and after a single move and compare against a physical cube or an online
 * simulator.
 */

#include "RubiksCube3dArray.h"
#include <iostream>

using namespace std;

// =========================================================================
// Constructor
// =========================================================================

RubiksCube3dArray::RubiksCube3dArray() {
    // Standard solved-state colour mapping:
    // Index 0=Up(W)  1=Left(G)  2=Front(R)  3=Right(B)  4=Back(O)  5=Down(Y)
    const char colors[6] = {'W', 'G', 'R', 'B', 'O', 'Y'};

    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                cube[face][row][col] = colors[face];
            }
        }
    }
}

// =========================================================================
// isSolved
// =========================================================================

bool RubiksCube3dArray::isSolved() const {
    for (int face = 0; face < 6; face++) {
        // The centre tile [1][1] never moves relative to the core.
        // It is the reference colour for this face.
        const char faceColor = cube[face][1][1];
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (cube[face][row][col] != faceColor) return false;
            }
        }
    }
    return true;
}

// =========================================================================
// print
// =========================================================================

void RubiksCube3dArray::print() const {
    // Helper lambda — prints a single row of a face without a trailing newline.
    auto printRow = [&](int face, int row) {
        for (int col = 0; col < 3; col++) {
            cout << cube[face][row][col] << " ";
        }
    };

    // Up face (indented to sit above the Front face)
    for (int row = 0; row < 3; row++) {
        cout << "      ";
        printRow(0, row);
        cout << "\n";
    }
    cout << "\n";

    // Middle band: Left | Front | Right | Back
    for (int row = 0; row < 3; row++) {
        printRow(1, row); cout << " ";  // Left
        printRow(2, row); cout << " ";  // Front
        printRow(3, row); cout << " ";  // Right
        printRow(4, row); cout << "\n"; // Back
    }
    cout << "\n";

    // Down face (indented to sit below the Front face)
    for (int row = 0; row < 3; row++) {
        cout << "      ";
        printRow(5, row);
        cout << "\n";
    }
    cout << "\n";
}

// =========================================================================
// getCorners
// =========================================================================

string RubiksCube3dArray::getCorners() const {
    string corners;
    corners.reserve(24); // 8 corners × 3 stickers

    // Corner sticker order within each triplet: (U/D sticker, adjacent sticker, adjacent sticker)
    // Canonical corner order: UBL, UBR, UFL, UFR, DBL, DBR, DFL, DFR

    corners += cube[0][0][0]; corners += cube[4][0][2]; corners += cube[1][0][0]; // UBL
    corners += cube[0][0][2]; corners += cube[4][0][0]; corners += cube[3][0][2]; // UBR
    corners += cube[0][2][0]; corners += cube[2][0][0]; corners += cube[1][0][2]; // UFL
    corners += cube[0][2][2]; corners += cube[2][0][2]; corners += cube[3][0][0]; // UFR
    corners += cube[5][2][0]; corners += cube[4][2][2]; corners += cube[1][2][0]; // DBL
    corners += cube[5][2][2]; corners += cube[4][2][0]; corners += cube[3][2][2]; // DBR
    corners += cube[5][0][0]; corners += cube[2][2][0]; corners += cube[1][2][2]; // DFL
    corners += cube[5][0][2]; corners += cube[2][2][2]; corners += cube[3][2][0]; // DFR

    return corners;
}

// =========================================================================
// rotateFaceClockwise
// =========================================================================

void RubiksCube3dArray::rotateFaceClockwise(int face) {
    // In-place 90° clockwise rotation:
    // Achieved by transposing and then reversing each row.
    // temp[j][2-i] = original[i][j]  →  equivalent to a CW rotation.
    char temp[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            temp[j][2 - i] = cube[face][i][j];
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cube[face][i][j] = temp[i][j];
        }
    }
}

// =========================================================================
// Individual face move implementations (all clockwise 90°)
// =========================================================================

void RubiksCube3dArray::u() {
    rotateFaceClockwise(0);
    char temp[3];

    // Shift the top row of the four side faces: Front → Left → Back → Right
    // (Viewed from above, a CW Up turn sweeps Front-row pieces to the right)
    for (int i = 0; i < 3; i++) temp[i]         = cube[4][0][i]; // cache Back
    for (int i = 0; i < 3; i++) cube[4][0][i]   = cube[1][0][i]; // Left  → Back
    for (int i = 0; i < 3; i++) cube[1][0][i]   = cube[2][0][i]; // Front → Left
    for (int i = 0; i < 3; i++) cube[2][0][i]   = cube[3][0][i]; // Right → Front
    for (int i = 0; i < 3; i++) cube[3][0][i]   = temp[i];       // Back  → Right
}

void RubiksCube3dArray::d() {
    rotateFaceClockwise(5);
    char temp[3];

    // Shift the bottom row of the four side faces: Front → Right → Back → Left
    // (Opposite rotation direction to U when viewed from the same side)
    for (int i = 0; i < 3; i++) temp[i]         = cube[4][2][i]; // cache Back
    for (int i = 0; i < 3; i++) cube[4][2][i]   = cube[3][2][i]; // Right → Back
    for (int i = 0; i < 3; i++) cube[3][2][i]   = cube[2][2][i]; // Front → Right
    for (int i = 0; i < 3; i++) cube[2][2][i]   = cube[1][2][i]; // Left  → Front
    for (int i = 0; i < 3; i++) cube[1][2][i]   = temp[i];       // Back  → Left
}

void RubiksCube3dArray::l() {
    rotateFaceClockwise(1);
    char temp[3];

    // Shift the left column of Up/Front/Down and the right column of Back.
    // Note: Back face is oriented inversely relative to the column cycle,
    //       so its indices run in reverse (2-i).
    for (int i = 0; i < 3; i++) temp[i]            = cube[0][i][0];     // cache Up-left col
    for (int i = 0; i < 3; i++) cube[0][i][0]      = cube[4][2 - i][2]; // Back-right (inv) → Up
    for (int i = 0; i < 3; i++) cube[4][2 - i][2]  = cube[5][i][0];     // Down-left (inv)  → Back
    for (int i = 0; i < 3; i++) cube[5][i][0]      = cube[2][i][0];     // Front-left → Down
    for (int i = 0; i < 3; i++) cube[2][i][0]      = temp[i];           // Up → Front
}

void RubiksCube3dArray::r() {
    rotateFaceClockwise(3);
    char temp[3];

    // Shift the right column of Up/Front/Down and the left column of Back.
    // Back face is again inverted relative to the column cycle.
    for (int i = 0; i < 3; i++) temp[i]            = cube[0][i][2];     // cache Up-right col
    for (int i = 0; i < 3; i++) cube[0][i][2]      = cube[2][i][2];     // Front-right → Up
    for (int i = 0; i < 3; i++) cube[2][i][2]      = cube[5][i][2];     // Down-right  → Front
    for (int i = 0; i < 3; i++) cube[5][i][2]      = cube[4][2 - i][0]; // Back-left (inv) → Down
    for (int i = 0; i < 3; i++) cube[4][2 - i][0]  = temp[i];           // Up → Back (inv)
}

void RubiksCube3dArray::f() {
    rotateFaceClockwise(2);
    char temp[3];

    // Shift the strips adjacent to the Front face:
    //   bottom row of Up → right col of Right → top row of Down (reversed) → left col of Left (reversed)
    for (int i = 0; i < 3; i++) temp[i]               = cube[0][2][i];     // cache Up-bottom row
    for (int i = 0; i < 3; i++) cube[0][2][i]         = cube[1][2 - i][2]; // Left-right (inv) → Up
    for (int i = 0; i < 3; i++) cube[1][2 - i][2]     = cube[5][0][2 - i]; // Down-top (inv)   → Left
    for (int i = 0; i < 3; i++) cube[5][0][2 - i]     = cube[3][i][0];     // Right-left → Down
    for (int i = 0; i < 3; i++) cube[3][i][0]         = temp[i];           // Up → Right
}

void RubiksCube3dArray::b() {
    rotateFaceClockwise(4);
    char temp[3];

    // Shift the strips adjacent to the Back face:
    //   top row of Up → right col of Right → bottom row of Down (reversed) → left col of Left (reversed)
    for (int i = 0; i < 3; i++) temp[i]               = cube[0][0][i];     // cache Up-top row
    for (int i = 0; i < 3; i++) cube[0][0][i]         = cube[3][i][2];     // Right-right → Up
    for (int i = 0; i < 3; i++) cube[3][i][2]         = cube[5][2][2 - i]; // Down-bottom (inv) → Right
    for (int i = 0; i < 3; i++) cube[5][2][2 - i]     = cube[1][2 - i][0]; // Left-left (inv) → Down
    for (int i = 0; i < 3; i++) cube[1][2 - i][0]     = temp[i];           // Up → Left (inv)
}

// =========================================================================
// performMove — router from Move enum to concrete functions
// =========================================================================

GenericRubiksCube& RubiksCube3dArray::performMove(Move move) {
    // Prime (CCW) = 3 × Clockwise.  Half-turn (2) = 2 × Clockwise.
    switch (move) {
        case Move::U: u(); break;
        case Move::U_PRIME: u(); u(); u(); break;
        case Move::U2: u(); u(); break;

        case Move::D: d(); break;
        case Move::D_PRIME: d(); d(); d(); break;
        case Move::D2: d(); d(); break;

        case Move::L: l(); break;
        case Move::L_PRIME: l(); l(); l(); break;
        case Move::L2: l(); l(); break;

        case Move::R: r(); break;
        case Move::R_PRIME: r(); r(); r(); break;
        case Move::R2: r(); r(); break;

        case Move::F: f(); break;
        case Move::F_PRIME: f(); f(); f(); break;
        case Move::F2: f(); f(); break;

        case Move::B: b(); break;
        case Move::B_PRIME: b(); b(); b(); break;
        case Move::B2: b(); b(); break;
    }
    return *this;
}

// =========================================================================
// getStateString
// =========================================================================

string RubiksCube3dArray::getStateString() const {
    string state;
    state.reserve(54); // 6 faces × 9 stickers — avoids repeated reallocation
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                state += cube[face][row][col];
            }
        }
    }
    return state;
}
