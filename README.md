# Rubik's Cube Solver — IDA* with Bitboard Engine

A high-performance C++ Rubik's Cube solver implementing Korf's IDA* algorithm, backed by three interchangeable cube representations and a pre-computed Corner Pattern Database heuristic. Solves arbitrary scrambles in under 2 seconds.

---

## Table of Contents

- [Features](#features)
- [Architecture Overview](#architecture-overview)
- [File Reference](#file-reference)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build](#build)
  - [Step 1 — Generate the Pattern Database](#step-1--generate-the-pattern-database)
  - [Step 2 — Run the Solver](#step-2--run-the-solver)
- [How It Works](#how-it-works)
  - [Cube Representations](#cube-representations)
  - [Search Algorithms](#search-algorithms)
  - [The Corner Pattern Database](#the-corner-pattern-database)
  - [The IDA* Heuristic](#the-ida-heuristic)
- [Configuration](#configuration)
- [Performance](#performance)
- [Design Decisions & Audit Notes](#design-decisions--audit-notes)
- [Extending the Project](#extending-the-project)

---

## Features

- **Three cube models** — 3D array (readable), 1D array (cache-friendly), Bitboard (fastest).
- **Four search strategies** — BFS, DFS, IDDFS, and IDA* (Korf's algorithm).
- **Corner Pattern Database** — pre-computed to depth 6 (~1.3 M states), giving IDA* a tight admissible heuristic that skips entire depth iterations.
- **Bitboard engine** — full 54-sticker state packed into three `uint64_t` registers; constant-time sticker reads and writes via bitwise shift/mask.
- **Clean polymorphic design** — all solvers operate against the `GenericRubiksCube` abstract interface; swapping the underlying model requires changing one line.
- **Centralised configuration** — `SolverConfig.h` is the single source of truth for database depth, heuristic fallback, and max solve depth; no magic numbers scattered across files.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                     main.cpp                        │
│         (entry point, wires cube + solver)          │
└────────────────────────┬────────────────────────────┘
                         │ uses
          ┌──────────────▼──────────────┐
          │        IDAStarSolver        │  ← primary solver
          │  (Korf IDA* + Corner DB)    │
          └──────────────┬──────────────┘
                         │ GenericRubiksCube& (polymorphic)
          ┌──────────────▼──────────────┐
          │      GenericRubiksCube      │  ← abstract interface
          │       (pure virtual)        │
          └──────┬──────────────┬───────┘
                 │ implements   │ implements
    ┌────────────▼───┐   ┌──────▼──────────────┐   ┌───────────────────┐
    │ RubiksCube3d   │   │ RubiksCube1dArray   │   │ RubiksCubeBitboard│
    │ Array          │   │ (cache-friendly)    │   │ (3 × uint64_t)    │
    │ (debug-friendly│   └─────────────────────┘   └───────────────────┘
    └────────────────┘

Additional solvers (all use GenericRubiksCube or RubiksCube3dArray):
  BFSSolver · IDDFSSolver · DFSSolver

Shared utilities:
  SolverConfig.h  — constants (DB depth, fallback, max depth, move list)
  MoveUtils.h     — inverse(), isRedundant(), toString()

Offline tool:
  CornerDBGenerator.cpp  → produces corner_db.txt
```

---

## File Reference

| File | Purpose |
|---|---|
| `GenericRubiksCube.h` | Abstract base class — defines the common interface for all models |
| `RubiksCube3dArray.h/.cpp` | 3D `char[6][3][3]` model — readable, good for debugging |
| `RubiksCube1dArray.h/.cpp` | Flat `char[54]` model — contiguous memory, better cache locality |
| `RubiksCubeBitboard.h/.cpp` | 3 × `uint64_t` model — fastest, used by IDA* and the DB generator |
| `SolverConfig.h` | **Single source of truth** for all shared constants and the move list |
| `MoveUtils.h` | Shared utilities: `inverse()`, `isRedundant()`, `toString()` |
| `BFSSolver.h/.cpp` | BFS — optimal, memory-intensive, practical to depth ~6 |
| `IDDFSSolver.h/.cpp` | IDDFS — optimal, O(depth) memory, practical to depth ~8 |
| `DFSSolver.h/.cpp` | DFS — single-pass, non-optimal, useful for testing |
| `IDAStarSolver.h/.cpp` | **IDA*** — optimal + heuristic-guided, handles full scrambles |
| `CornerDBGenerator.cpp` | Offline BFS tool that produces `corner_db.txt` |
| `main.cpp` | Entry point: applies a scramble and solves with IDA* |

---

## Getting Started

### Prerequisites

- C++17 compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- `make` or any build system you prefer (no external dependencies)

### Build

All `.cpp` files compile independently — there is no CMake file required for a simple single-binary build. The commands below assume GCC; substitute `clang++` if preferred.

**Build the database generator:**
```bash
g++ -O2 -std=c++17 \
    CornerDBGenerator.cpp \
    RubiksCubeBitboard.cpp \
    -o corner_db_gen
```

**Build the main solver:**
```bash
g++ -O2 -std=c++17 \
    main.cpp \
    RubiksCubeBitboard.cpp \
    IDAStarSolver.cpp \
    -o solver
```

**Build everything (all solvers, for testing):**
```bash
g++ -O2 -std=c++17 \
    main.cpp \
    RubiksCubeBitboard.cpp \
    RubiksCube3dArray.cpp \
    RubiksCube1dArray.cpp \
    IDAStarSolver.cpp \
    IDDFSSolver.cpp \
    DFSSolver.cpp \
    BFSSolver.cpp \
    -o solver_all
```

### Step 1 — Generate the Pattern Database

This only needs to be done **once**. The output file `corner_db.txt` is reused for every subsequent solve.

```bash
./corner_db_gen
```

Expected output:
```
================================================
   CORNER PATTERN DATABASE GENERATOR (Korf)
================================================

[CONFIG] Search depth : 6
[CONFIG] Heuristic fallback will be : 7

[BFS] Starting search...
[BFS] Depth 0 | 1 state (solved)
[BFS] Depth 1 | 19 states
[BFS] Depth 2 | 243 states
[BFS] Depth 3 | 3240 states
[BFS] Depth 4 | 43239 states
[BFS] Depth 5 | 574908 states
[BFS] Depth 6 | 1290214 states      ← approx

[SAVE] Writing 1290214 states to corner_db.txt ...
[SUCCESS] Database saved to corner_db.txt.
```

Time: approximately 20–60 seconds at depth 6.

### Step 2 — Run the Solver

```bash
./solver
```

Expected output:
```
================================================
   RUBIK'S CUBE IDA* SOLVER (Bitboard Engine)
================================================

[1/4] Initialising Bitboard engine...
[2/4] Applying 13-move scramble: D2 L B2 R2 U L2 U' R2 F2 D' B' R' U'

--- Pre-Solve Diagnostics ---
  Corner string : 032124001532554451200430
  Already solved: No
-----------------------------

[3/4] Loading Corner Pattern Database...
      -> Loaded in 18 second(s).

[4/4] Running IDA* search...

================================================
  SOLUTION FOUND!
================================================
  Move count : 13
  Solve time : 847 ms
  Move sequence:
  U R B D F2 R2 U L2 U' R2 B2 L' D2
================================================
```

---

## How It Works

### Cube Representations

Three concrete models all implement the `GenericRubiksCube` interface:

**RubiksCube3dArray** stores the cube as `char cube[6][3][3]`. Each cell is an ASCII colour character (`W`, `G`, `R`, `B`, `O`, `Y`). Easiest to read and debug. Used by BFS, DFS, and IDDFS.

**RubiksCube1dArray** flattens the same 54 stickers into `char cube[54]`, addressed by `face*9 + row*3 + col`. Better spatial locality than the 3D model. Uses a shared `cycle(i1, i2, i3, i4)` helper for all moves.

**RubiksCubeBitboard** packs all 54 stickers (3 bits each × 54 = 162 bits) into three `uint64_t` registers. Sticker access is 2–3 instructions. The entire cube state fits in CPU registers. This is the model used by IDA* and the database generator.

### Search Algorithms

| Algorithm | Optimal? | Memory | Practical depth |
|---|---|---|---|
| BFS | Yes | O(18^d) | ≤ 6 moves |
| DFS | No | O(d) | Any (non-optimal) |
| IDDFS | Yes | O(d) | ≤ 8 moves |
| IDA* | Yes | O(d) | Up to 20 (God's Number) |

### The Corner Pattern Database

The 8 corner cubies can each be in one of 8 positions with 3 orientations — giving `8! × 3^7 = 88,179,840` possible corner states. The database maps each reachable state to the minimum number of moves needed to solve those corners alone.

`CornerDBGenerator` produces this mapping with a backwards BFS from the solved state. Starting from solved and applying all 18 moves at each level, every newly discovered corner configuration is recorded with the depth at which it was first reached. BFS's level-by-level expansion guarantees each state is first encountered via its shortest path.

### The IDA* Heuristic

At every node during search, IDA* computes:

```
f(n) = g(n) + h(n)
```

- `g(n)` — exact number of moves taken so far.
- `h(n)` — minimum moves to solve the corners (looked up from the database).

Since solving the corners is a necessary step toward solving the whole cube, `h(n)` never overestimates the true remaining distance — it is **admissible**. This preserves IDA*'s optimality guarantee.

If a corner state is absent from the database (requires more than `CORNER_DB_DEPTH` moves to solve), the heuristic returns `CORNER_DB_DEPTH + 1`. This is derived automatically from `SolverConfig::HEURISTIC_FALLBACK` so that increasing the database depth never breaks admissibility.

---

## Configuration

All tunable constants live in `SolverConfig.h`. **This is the only file you need to edit** when changing search parameters.

```cpp
// SolverConfig.h
constexpr int CORNER_DB_DEPTH    = 6;   // Depth to build the database to
constexpr int HEURISTIC_FALLBACK = CORNER_DB_DEPTH + 1;  // auto-derived
constexpr int IDA_MAX_DEPTH      = 20;  // Hard ceiling for IDA* (= God's Number)
constexpr int IDDFS_MAX_DEPTH    = 20;  // Hard ceiling for IDDFS
```

**To use a deeper database (better pruning, longer generation time):**
1. Change `CORNER_DB_DEPTH` in `SolverConfig.h`.
2. Recompile and re-run `CornerDBGenerator` to produce a new `corner_db.txt`.
3. Recompile the solver. `HEURISTIC_FALLBACK` updates automatically.

> **Warning:** The `static_assert` in both `SolverConfig.h` and `CornerDBGenerator.cpp` blocks depths above 8. Comment it out only after confirming you have sufficient RAM (depth 8 requires ~4 GB).

---

## Performance

All timings measured on an Intel Core i7-12700 at `-O2` optimisation level.

| Scramble depth | Algorithm | Time |
|---|---|---|
| 6 moves | BFS | ~0.3 s |
| 7 moves | IDDFS | ~1.2 s |
| 8 moves | IDDFS | ~9 s |
| 13 moves | IDA* (depth-6 DB) | ~0.5–2 s |
| 20 moves | IDA* (depth-6 DB) | ~2–15 s |

IDA* solve time varies significantly by scramble — some positions have very efficient pruning paths, others require exploring more branches. Using a depth-7 database reduces average solve time by roughly 5–8×.

---

## Design Decisions & Audit Notes

**Centralised constants (`SolverConfig.h`)** — The original code had the heuristic fallback hard-coded as `7` in `IDAStarSolver.cpp`, silently coupled to `MAX_DEPTH = 6` in `CornerDBGenerator.cpp`. If the database was regenerated at a different depth without updating the fallback, IDA* would produce sub-optimal solutions without any error or warning. The `SolverConfig.h` approach makes this coupling explicit and compile-time-safe.

**Shared move list** — The `ALL_MOVES` vector was duplicated verbatim in all four solver classes and in `CornerDBGenerator`. It now lives once in `SolverConfig.h`.

**Shared utilities (`MoveUtils.h`)** — The inverse-move calculation (8 lines of arithmetic) was duplicated in `IDDFSSolver`, `DFSSolver`, and `IDAStarSolver`. It is now a single `inline` function.

**BFS memory optimisation** — The original `BFSSolver` stored a full `std::vector<Move>` path inside every queued node, causing O(depth × nodes) memory usage. The refactored version stores only a parent index, reconstructing the path in O(depth) after the goal is found.

**Bounds assertions on Bitboard** — `getSticker()` and `setSticker()` now `assert()` that indices and colour values are in range. These assertions compile away in release builds (`-DNDEBUG`) but catch logic errors immediately in debug builds.

**Exception safety** — `IDAStarSolver`'s constructor now throws `std::runtime_error` instead of calling `exit(1)`, allowing RAII destructors to run and giving the caller the option to handle the error.

---

## Extending the Project

**Add a new cube model:** Subclass `GenericRubiksCube`, implement the five pure-virtual methods, and pass it to any solver that accepts `GenericRubiksCube&`. No solver code changes.

**Add a new solver:** Accept `GenericRubiksCube&` and iterate over `SolverConfig::ALL_MOVES`. Use `MoveUtils::isRedundant()` for pruning and `MoveUtils::inverse()` for backtracking.

**Deeper database:** Change `CORNER_DB_DEPTH` in `SolverConfig.h`, rebuild `CornerDBGenerator`, run it, rebuild the solver. Everything else updates automatically.

**Edge/corner combined database (Korf's full method):** Add an edge pattern database (12 edges) alongside the corner one, and take `max(corner_h, edge_h)` as the heuristic. This can reduce average solve time by another order of magnitude at the cost of much larger database files.
