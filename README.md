# A simple console application using C++ for Vanilla Option using the Black-Scholes model

run using `g++ main.cpp black.cpp -o main ; if ($?) { .\main }` in the terminal

---

## Structure
| File       | Role                                        |
| ---------- | ------------------------------------------- |
| black.h    | Declarations for normalCDF and BlackScholes |
| black.cpp  | Black-Scholes implementation                |
| main.cpp   | Main program: I/O, parsing, pricing loop    |
| trades.txt | Input trade data (semicolon-delimited)      |

## Design Points

### black.cpp — Black-Scholes formula:

- d1 = (ln(S/K) + (r + σ²/2)·T) / (σ√T)
- d2 = d1 − σ√T
- Call PV = S·N(d1) − K·e^(−rT)·N(d2), scaled by notional
- Put PV = K·e^(−rT)·N(−d2) − S·N(−d1), scaled by notional
- normalCDF uses std::erfc (C++ standard, no external dependency)
- Throws std::invalid_argument for non-positive spot/strike/vol/expiry

### main.cpp — Three key functions:

- split(line, delimiter) — tokenises a string by any delimiter, trims whitespace
- readLine(ifstream&, string&) — reads the next non-blank line
- loadTradeFromFile — reads header, parses each row with split, catches stod/stoi failures (e.g. trade 2 has 3k as notional — it is skipped with a [WARN] message)
- saveResults — writes a formatted table to result.txt

### Error handling operates at two levels:

- Parse errors (bad data in file) — caught per-row in loadTradeFromFile, prints [WARN] and skips that row
- Pricing errors (invalid model inputs) — caught per-trade in the main loop via invalid_argument, records the error message and marks the result N/A in result.txt
