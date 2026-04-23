#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>

#include "black.h"

using namespace std;

struct OptionTrade
{
    int        tradeId;
    double     notional;
    double     strike;
    bool       isCall;
    double     expiry;
};

// Splits `line` by `delimiter` and returns each token as a string.
// Tokens are trimmed of surrounding whitespace.
vector<string> split(const string& line, char delimiter)
{
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (getline(ss, token, delimiter))
    {
        // Trim leading/trailing spaces
        size_t start = token.find_first_not_of(" \t\r\n");
        size_t end   = token.find_last_not_of(" \t\r\n");
        if (start != string::npos)
            tokens.push_back(token.substr(start, end - start + 1));
        else
            tokens.push_back("");
    }
    return tokens;
}

// Reads a single line from `in` and stores it in `line`.
// Returns true if a non-empty line was read, false at EOF.
bool readLine(ifstream& in, string& line)
{
    while (getline(in, line))
    {
        if (!line.empty() && line.find_first_not_of(" \t\r\n") != string::npos)
            return true;
    }
    return false;
}

// Parses trades from `filePath` and appends them to `tradesSet`.
// The expected format (semicolon-delimited, first row is header):
//   tradeId;notional;strike;isCall;expiry;
// Skips rows that cannot be parsed and prints a warning.
void loadTradeFromFile(vector<OptionTrade>& tradesSet, const string& filePath)
{
    ifstream in(filePath);
    if (!in.is_open())
        throw runtime_error("Cannot open trade file: " + filePath);

    string line;
    // Skip header row
    if (!readLine(in, line))
        throw runtime_error("Trade file is empty: " + filePath);

    int lineNumber = 1;
    while (readLine(in, line))
    {
        ++lineNumber;
        vector<string> fields = split(line, ';');

        // We expect at least 5 fields (trailing semicolon adds an empty 6th — that's fine)
        if (fields.size() < 5)
        {
            cerr << "[WARN] Line " << lineNumber << ": expected 5 fields, got "
                 << fields.size() << " — skipping: \"" << line << "\"\n";
            continue;
        }

        try
        {
            OptionTrade t;
            t.tradeId  = stoi(fields[0]);
            t.notional = stod(fields[1]);   // throws if not a valid number (e.g. "3k")
            t.strike   = stod(fields[2]);
            t.isCall   = (fields[3] == "true" || fields[3] == "1");
            t.expiry   = stod(fields[4]);

            tradesSet.push_back(t);
        }
        catch (const invalid_argument& e)
        {
            cerr << "[WARN] Line " << lineNumber << ": invalid field value — skipping. ("
                 << e.what() << ")\n";
        }
        catch (const out_of_range& e)
        {
            cerr << "[WARN] Line " << lineNumber << ": field value out of range — skipping. ("
                 << e.what() << ")\n";
        }
    }

    if (tradesSet.empty())
        throw runtime_error("No valid trades were loaded from: " + filePath);
}

// Writes trade results to `filePath`.
// `errors` contains an error message for failed trades (empty string = success).
void saveResults(const string& filePath,
                 const vector<OptionTrade>& trades,
                 const vector<double>& pvs,
                 const vector<string>& errors)
{
    ofstream out(filePath);
    if (!out.is_open())
        throw runtime_error("Cannot open output file: " + filePath);

    out << fixed << setprecision(4);
    out << left
        << setw(10) << "TradeId"
        << setw(12) << "Notional"
        << setw(10) << "Strike"
        << setw(8)  << "Type"
        << setw(10) << "Expiry"
        << setw(18) << "PV"
        << "Status\n";
    out << string(80, '-') << "\n";

    for (size_t i = 0; i < trades.size(); ++i)
    {
        const auto& t = trades[i];
        out << setw(10) << t.tradeId
            << setw(12) << t.notional
            << setw(10) << t.strike
            << setw(8)  << (t.isCall ? "Call" : "Put")
            << setw(10) << t.expiry;

        if (errors[i].empty())
            out << setw(18) << pvs[i] << "OK\n";
        else
            out << setw(18) << "N/A" << "ERROR: " << errors[i] << "\n";
    }

    out << string(80, '-') << "\n";
    cout << "Results saved to: " << filePath << "\n";
}

int main()
{
    cout << "=== Vanilla Option PV — Black-Scholes Pricer ===\n\n";

    // Market data (shared across all trades)
    const double spot = 100.0;
    const double vol  = 0.20;
    const double rate = 0.045;

    // --- Load trades ---
    vector<OptionTrade> tradesSet;
    const string tradeFile  = "trades.txt";
    const string resultFile = "result.txt";

    try
    {
        loadTradeFromFile(tradesSet, tradeFile);
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] Failed to load trades: " << e.what() << "\n";
        return 1;
    }

    cout << "Loaded " << tradesSet.size() << " trade(s) from " << tradeFile << "\n\n";

    // --- Price each trade ---
    vector<double> pvResults(tradesSet.size(), 0.0);
    vector<string> errors(tradesSet.size());

    for (size_t i = 0; i < tradesSet.size(); ++i)
    {
        const auto& t = tradesSet[i];
        try
        {
            pvResults[i] = BlackScholes(t.notional, t.strike, t.expiry,
                                        spot, vol, rate, t.isCall);
            cout << "Trade " << t.tradeId
                 << " (" << (t.isCall ? "Call" : "Put") << ")"
                 << "  PV = " << fixed << setprecision(4) << pvResults[i] << "\n";
        }
        catch (const invalid_argument& e)
        {
            errors[i] = e.what();
            cerr << "[ERROR] Trade " << t.tradeId << ": " << e.what() << "\n";
        }
        catch (const exception& e)
        {
            errors[i] = e.what();
            cerr << "[ERROR] Trade " << t.tradeId << ": " << e.what() << "\n";
        }
    }

    // --- Save results ---
    try
    {
        saveResults(resultFile, tradesSet, pvResults, errors);
    }
    catch (const exception& e)
    {
        cerr << "[ERROR] Failed to save results: " << e.what() << "\n";
        return 1;
    }

    cout << "\n=== Done ===\n";
    return 0;
}
