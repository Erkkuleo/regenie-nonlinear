/*

   This file is part of the regenie software package.

   Copyright (c) 2020-2024 Joelle Mbatchou, Andrey Ziyatdinov & Jonathan Marchini

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

*/

#ifndef SUNRISE_H
#define SUNRISE_H

#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
// SunriseCalculator: NOAA solar position algorithm
//
// Based on the NOAA spreadsheet equations:
//   https://gml.noaa.gov/grad/solcalc/calcdetails.html
// This is the same algorithm used by the sunwait project
//   (https://github.com/risacher/sunwait).
//
// All public methods return times in decimal hours UTC.
// Use NaN results to detect polar night; +inf for polar day (no sunrise).
// ---------------------------------------------------------------------------
class SunriseCalculator {
public:

    // -----------------------------------------------------------------------
    // Compute sunrise time (UTC decimal hours) for:
    //   lat_deg : latitude  in decimal degrees, N positive
    //   lon_deg : longitude in decimal degrees, E positive
    //   doy     : day-of-year, 1-based (Jan 1 = 1)
    //   year    : 4-digit year (used for Julian Date calculation)
    //
    // Returns:
    //   decimal hours UTC on success (e.g. 4.5 = 04:30 UTC)
    //   NaN  if polar night (sun never rises)
    //   -inf if polar day  (sun never sets / always above horizon)
    // -----------------------------------------------------------------------
    static double computeSunrise(double lat_deg, double lon_deg, int doy, int year) {
        return computeSolarEvent(lat_deg, lon_deg, doy, year, /*sunrise=*/true);
    }

    // -----------------------------------------------------------------------
    // Compute sunset time (UTC decimal hours).  Same edge-case returns as above.
    // -----------------------------------------------------------------------
    static double computeSunset(double lat_deg, double lon_deg, int doy, int year) {
        return computeSolarEvent(lat_deg, lon_deg, doy, year, /*sunrise=*/false);
    }

private:

    static constexpr double DEG2RAD = M_PI / 180.0;
    static constexpr double RAD2DEG = 180.0 / M_PI;

    // Solar zenith angle at which we define sunrise/sunset (degrees).
    // 90.833° = 90° (horizon) + 0.833° (atmospheric refraction + solar disk radius)
    static constexpr double ZENITH = 90.833;

    // -----------------------------------------------------------------------
    // Internal: compute fractional year γ (radians) from doy.
    // NOAA formula uses 1-based doy; noon UTC assumed.
    // -----------------------------------------------------------------------
    static double fractionalYear(int doy, int year) {
        // Days in year (leap-year aware)
        int days_in_year = isLeapYear(year) ? 366 : 365;
        // γ = 2π/N * (doy - 1 + (12-12)/24)  [noon UTC → (hour-12)/24 = 0]
        return (2.0 * M_PI / days_in_year) * (doy - 1.0);
    }

    // -----------------------------------------------------------------------
    // Equation of time in minutes (NOAA approximation).
    // -----------------------------------------------------------------------
    static double equationOfTime(double gamma) {
        return 229.18 * (0.000075
                         + 0.001868 * std::cos(gamma)
                         - 0.032077 * std::sin(gamma)
                         - 0.014615 * std::cos(2.0 * gamma)
                         - 0.04089  * std::sin(2.0 * gamma));
    }

    // -----------------------------------------------------------------------
    // Solar declination angle in radians (NOAA approximation).
    // -----------------------------------------------------------------------
    static double solarDeclination(double gamma) {
        return 0.006918
               - 0.399912 * std::cos(gamma)
               + 0.070257 * std::sin(gamma)
               - 0.006758 * std::cos(2.0 * gamma)
               + 0.000907 * std::sin(2.0 * gamma)
               - 0.002697 * std::cos(3.0 * gamma)
               + 0.00148  * std::sin(3.0 * gamma);
    }

    // -----------------------------------------------------------------------
    // Core: compute sunrise or sunset in UTC decimal hours.
    // -----------------------------------------------------------------------
    static double computeSolarEvent(double lat_deg, double lon_deg,
                                    int doy, int year, bool is_sunrise) {
        double lat_rad = lat_deg * DEG2RAD;
        double gamma   = fractionalYear(doy, year);

        double eqtime = equationOfTime(gamma);       // minutes
        double decl   = solarDeclination(gamma);     // radians

        // Hour-angle argument: cos(zenith) / (cos(lat)*cos(decl)) - tan(lat)*tan(decl)
        double cos_zenith = std::cos(ZENITH * DEG2RAD);
        double cos_lat    = std::cos(lat_rad);
        double cos_decl   = std::cos(decl);
        double tan_lat    = std::tan(lat_rad);
        double tan_decl   = std::tan(decl);

        double ha_arg = cos_zenith / (cos_lat * cos_decl) - tan_lat * tan_decl;

        if (ha_arg > 1.0) {
            // Polar night: sun never reaches the horizon
            return std::numeric_limits<double>::quiet_NaN();
        }
        if (ha_arg < -1.0) {
            // Polar day: sun never sets
            return -std::numeric_limits<double>::infinity();
        }

        double ha_rad = std::acos(ha_arg);  // sunrise hour angle (radians, positive)
        double ha_deg = ha_rad * RAD2DEG;   // degrees

        // NOAA formula: minutes UTC from midnight
        // sunrise: 720 - 4*(lon + ha_deg) - eqtime
        // sunset:  720 - 4*(lon - ha_deg) - eqtime
        double sign   = is_sunrise ? 1.0 : -1.0;
        double minutes_utc = 720.0 - 4.0 * (lon_deg + sign * ha_deg) - eqtime;

        // Convert to hours UTC, wrap to [0, 24)
        double hours_utc = minutes_utc / 60.0;
        hours_utc = std::fmod(hours_utc, 24.0);
        if (hours_utc < 0.0) hours_utc += 24.0;

        return hours_utc;
    }

    static bool isLeapYear(int year) {
        return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
    }
};

#endif
