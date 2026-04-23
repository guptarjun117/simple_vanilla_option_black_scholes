#include "black.h"
#include <cmath>
#include <stdexcept>
#include <string>

// Approximation via complementary error function (accurate to ~1e-15)
double normalCDF(double x)
{
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double BlackScholes(double notional, double strike, double expiry,
                    double spot, double vol, double rate, bool isCall)
{
    if (spot <= 0.0)
        throw std::invalid_argument("Spot price must be positive, got: " + std::to_string(spot));
    if (strike <= 0.0)
        throw std::invalid_argument("Strike must be positive, got: " + std::to_string(strike));
    if (vol <= 0.0)
        throw std::invalid_argument("Volatility must be positive, got: " + std::to_string(vol));
    if (expiry <= 0.0)
        throw std::invalid_argument("Expiry must be positive, got: " + std::to_string(expiry));
    if (notional == 0.0)
        throw std::invalid_argument("Notional must be non-zero");

    double sqrtT = std::sqrt(expiry);
    double d1 = (std::log(spot / strike) + (rate + 0.5 * vol * vol) * expiry) / (vol * sqrtT);
    double d2 = d1 - vol * sqrtT;
    double discount = std::exp(-rate * expiry);

    double unitPV;
    if (isCall)
        unitPV = spot * normalCDF(d1) - strike * discount * normalCDF(d2);
    else
        unitPV = strike * discount * normalCDF(-d2) - spot * normalCDF(-d1);

    return notional * unitPV;
}
