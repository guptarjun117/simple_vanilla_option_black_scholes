#pragma once

// normalCDF: standard normal cumulative distribution function
double normalCDF(double x);

// BlackScholes: computes call or put present value
//   notional : contract notional
//   strike   : option strike price (K)
//   expiry   : time to expiry in years (T)
//   spot     : current underlying price (S)
//   vol      : annualised volatility (sigma)
//   rate     : continuous risk-free rate (r)
//   isCall   : true for call, false for put
// Throws std::invalid_argument if any input is out of range.
double BlackScholes(double notional, double strike, double expiry,
                    double spot, double vol, double rate, bool isCall);
