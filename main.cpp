/**
 * @file main.cpp
 * @brief Entry point demonstrating the full IDA* solve pipeline.
 *
 * Pipeline:
 *   1. Construct a RubiksCubeBitboard in the solved state.
 *   2. Apply a known scramble sequence to produce a specific scrambled position.
 *   3. Load the Corner Pattern Database into IDAStarSolver.
 *   4. Run IDA* and time the solve.
 *   5. Print the solution move sequence and performance metrics.
 *
 * @par Prerequisites
 *   Run CornerDBGenerator once to produce corner_db.txt before executing this
 *   program. IDAStarSolver will throw a std::runtime_error if the file is missing.
 *
 * @par Compile example
 * @code
 *   g++ -O2 -std=c++17 main.cpp RubiksCubeBitboard.cpp IDAStarSolver.cpp -o solver
 *   ./solver
 * @endcode
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <stdexcept>
#include "RubiksCubeBitboard.h"
#include "IDAStarSolver.h"
#include "MoveUtils.h"

using namespace std;
using namespace chrono;

int main() {
    cout << "================================================\n";
    cout << "   RUBIK'S CUBE IDA* SOLVER (Bitboard Engine)  \n";
    cout << "================================================\n\n";

    // -----------------------------------------------------------------------
    // Step 1: Construct the cube and apply the scramble
    // -----------------------------------------------------------------------

    cout << "[1/4] Initialising Bitboard engine...\n";
    RubiksCubeBitboard cube;

    cout << "[2/4] Applying 13-move scramble: D2 L B2 R2 U L2 U' R2 F2 D' B' R' U'\n";
    cube.performMove(GenericRubiksCube::Move::D2)
        .performMove(GenericRubiksCube::Move::L)
        .performMove(GenericRubiksCube::Move::B2)
        .performMove(GenericRubiksCube::Move::R2)
        .performMove(GenericRubiksCube::Move::U)
        .performMove(GenericRubiksCube::Move::L2)
        .performMove(GenericRubiksCube::Move::U_PRIME)
        .performMove(GenericRubiksCube::Move::R2)
        .performMove(GenericRubiksCube::Move::F2)
        .performMove(GenericRubiksCube::Move::D_PRIME)
        .performMove(GenericRubiksCube::Move::B_PRIME)
        .performMove(GenericRubiksCube::Move::R_PRIME)
        .performMove(GenericRubiksCube::Move::U_PRIME);

    // Sanity check before attempting the solve
    cout << "\n--- Pre-Solve Diagnostics ---\n";
    cout << "  Corner string : " << cube.getCorners() << "\n";
    cout << "  Already solved: " << (cube.isSolved() ? "Yes" : "No") << "\n";
    cout << "-----------------------------\n\n";

    // -----------------------------------------------------------------------
    // Step 2: Load the Corner Pattern Database
    // -----------------------------------------------------------------------

    cout << "[3/4] Loading Corner Pattern Database...\n";
    cout << "      (First run may take 15–45 seconds depending on DB size)\n";

    IDAStarSolver* solver = nullptr;
    try {
        auto load_start = high_resolution_clock::now();
        solver = new IDAStarSolver("corner_db.txt");
        auto load_end   = high_resolution_clock::now();

        cout << "      -> Loaded in "
             << duration_cast<seconds>(load_end - load_start).count()
             << " second(s).\n\n";
    }
    catch (const runtime_error& e) {
        cerr << e.what() << "\n";
        cerr << "Tip: compile and run CornerDBGenerator first.\n";
        return 1;
    }

    // -----------------------------------------------------------------------
    // Step 3: Run IDA*
    // -----------------------------------------------------------------------

    cout << "[4/4] Running IDA* search...\n";

    auto solve_start = high_resolution_clock::now();
    vector<GenericRubiksCube::Move> solution = solver->solve(cube);
    auto solve_end   = high_resolution_clock::now();

    delete solver;

    // -----------------------------------------------------------------------
    // Step 4: Print results
    // -----------------------------------------------------------------------

    cout << "\n================================================\n";
    if (solution.empty()) {
        cout << "  NO SOLUTION FOUND within depth limit.\n";
    } else {
        cout << "  SOLUTION FOUND!\n";
        cout << "================================================\n";
        cout << "  Move count : " << solution.size() << "\n";
        cout << "  Solve time : "
             << duration_cast<milliseconds>(solve_end - solve_start).count()
             << " ms\n";

        cout << "\n  Move sequence:\n  ";
        for (size_t i = 0; i < solution.size(); i++) {
            cout << MoveUtils::toString(solution[i]);
            if (i + 1 < solution.size()) cout << " ";
        }
        cout << "\n";
    }
    cout << "================================================\n";

    return 0;
}
