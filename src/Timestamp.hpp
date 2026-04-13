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

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <cmath>
#include <cstring>

// POSIX timegm: treat std::tm as UTC and return time_t without local-TZ influence.
// Available on Linux/macOS (same POSIX requirement as gmtime_r already used in this file).
#ifdef _WIN32
// Windows equivalent
static inline time_t timegm(struct tm* tm) { return _mkgmtime(tm); }
#endif

class TimestampCalculator {
public:

    // -------------------------------------------------------------------------
    // Helper: strip trailing TZ suffix from an ISO 8601 string and return the
    // UTC offset in hours.
    //
    // Supported suffixes:  "Z"  "+HH:MM"  "-HH:MM"
    // Returns true if a TZ suffix was found (offset_hours is set).
    // Returns false for a naive (no-TZ) string (offset_hours left unchanged).
    // datetime_part receives the string with the suffix removed.
    // -------------------------------------------------------------------------
    static bool parseTZOffset(const std::string& iso,
                               std::string& datetime_part,
                               double& offset_hours) {
        if (iso.empty()) {
            datetime_part = iso;
            return false;
        }

        // "Z" suffix → UTC
        if (iso.back() == 'Z') {
            datetime_part = iso.substr(0, iso.size() - 1);
            offset_hours = 0.0;
            return true;
        }

        // "+HH:MM" or "-HH:MM" suffix (6 chars)
        size_t len = iso.size();
        if (len >= 6) {
            char sign = iso[len - 6];
            char colon = iso[len - 3];
            if ((sign == '+' || sign == '-') && colon == ':') {
                // Validate that the offset digits are numeric
                bool digits_ok = true;
                for (size_t k : {len-5, len-4, len-2, len-1}) {
                    if (iso[k] < '0' || iso[k] > '9') { digits_ok = false; break; }
                }
                if (digits_ok) {
                    int tz_hh = (iso[len-5] - '0') * 10 + (iso[len-4] - '0');
                    int tz_mm = (iso[len-2] - '0') * 10 + (iso[len-1] - '0');
                    offset_hours = tz_hh + tz_mm / 60.0;
                    if (sign == '-') offset_hours = -offset_hours;
                    datetime_part = iso.substr(0, len - 6);
                    return true;
                }
            }
        }

        datetime_part = iso;
        return false;
    }

    // -------------------------------------------------------------------------
    // Helper: compute day-of-year (0-based, Jan 1 = 0) from a std::tm whose
    // tm_year, tm_mon, tm_mday fields have been populated by std::get_time.
    // Does NOT require mktime() — works independently of local TZ.
    // -------------------------------------------------------------------------
    static int doy_from_tm(const std::tm& tm) {
        static const int days_before_month[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
        int year = tm.tm_year + 1900;
        bool leap = (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
        int doy = days_before_month[tm.tm_mon] + tm.tm_mday - 1;
        if (leap && tm.tm_mon >= 2) doy++;
        return doy;
    }

    // -------------------------------------------------------------------------
    // Parse ISO 8601 timestamp to UTC time_t.
    // Offset-free strings are treated as UTC (uses timegm, not mktime).
    // Strings with TZ offset are converted to UTC by subtracting the offset.
    // -------------------------------------------------------------------------
    static std::chrono::system_clock::time_point parseISO(const std::string& iso_timestamp) {
        std::string dt_part;
        double tz_offset_hours = 0.0;
        bool has_tz = parseTZOffset(iso_timestamp, dt_part, tz_offset_hours);

        std::tm tm = {};
        std::istringstream ss(dt_part);

        // Try full ISO format: YYYY-MM-DDTHH:MM:SS
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            // Try date-only: YYYY-MM-DD
            ss.clear();
            ss.str(dt_part);
            ss >> std::get_time(&tm, "%Y-%m-%d");
            if (ss.fail()) {
                throw std::runtime_error("Invalid ISO timestamp format: " + iso_timestamp +
                                         ". Expected format: YYYY-MM-DDTHH:MM:SS or YYYY-MM-DD");
            }
        }

        // timegm treats tm as UTC → correct for both naive (assumed UTC) and TZ-bearing strings
        tm.tm_isdst = 0;
        time_t t = timegm(&tm);

        // For strings with explicit TZ offset: subtract offset to get UTC
        // e.g. "09:30+02:00" → UTC = 09:30 − 2h = 07:30
        if (has_tz && tz_offset_hours != 0.0) {
            t -= static_cast<time_t>(tz_offset_hours * 3600.0);
        }

        return std::chrono::system_clock::from_time_t(t);
    }

    // -------------------------------------------------------------------------
    // Calculate hours between two ISO timestamps (UTC difference).
    // -------------------------------------------------------------------------
    static double hoursBetween(const std::string& from_iso, const std::string& to_iso) {
        auto from_time = parseISO(from_iso);
        auto to_time   = parseISO(to_iso);
        auto duration  = std::chrono::duration_cast<std::chrono::seconds>(to_time - from_time);
        return static_cast<double>(duration.count()) / 3600.0;
    }

    static double hoursFromNow(const std::string& iso_timestamp) {
        auto target_time = parseISO(iso_timestamp);
        auto now         = std::chrono::system_clock::now();
        auto duration    = std::chrono::duration_cast<std::chrono::seconds>(now - target_time);
        return static_cast<double>(duration.count()) / 3600.0;
    }

    static double hoursTo(const std::string& reference_iso, const std::string& target_iso) {
        return hoursBetween(reference_iso, target_iso);
    }

    // -------------------------------------------------------------------------
    // Extract 4-digit calendar year from ISO 8601 string.
    // -------------------------------------------------------------------------
    static int extractYear(const std::string& iso_timestamp) {
        std::string dt_part;
        double tz_offset = 0.0;
        parseTZOffset(iso_timestamp, dt_part, tz_offset);
        std::tm tm = {};
        std::istringstream ss(dt_part);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            ss.clear(); ss.str(dt_part);
            ss >> std::get_time(&tm, "%Y-%m-%d");
        }
        return tm.tm_year + 1900;
    }

    // -------------------------------------------------------------------------
    // Extract LOCAL time-of-day in decimal hours [0, 24).
    //
    // For strings WITH a TZ offset (e.g. "2024-03-15T09:30:00+02:00"):
    //   returns the wall-clock hour from the string (9.5) — the local time the
    //   participant experienced — regardless of tz_shift_hours.
    //
    // For offset-free / "Z" strings (treated as UTC):
    //   returns UTC hour + tz_shift_hours (mod 24).
    //   tz_shift_hours comes from --timestamp-tz flag (default 0.0).
    //
    // e.g. "2024-03-15T09:30:00"  + tz_shift=+2.0 → 11.5 (UTC+2 local time)
    //      "2024-03-15T09:30:00Z" + tz_shift=0.0  → 9.5
    //      "2024-03-15T09:30:00+02:00"             → 9.5 (local time in string)
    // -------------------------------------------------------------------------
    static double extractTOD(const std::string& iso_timestamp,
                              double tz_shift_hours = 0.0) {
        std::string dt_part;
        double tz_offset_hours = 0.0;
        bool has_explicit_tz = parseTZOffset(iso_timestamp, dt_part, tz_offset_hours);

        std::tm tm = {};
        std::istringstream ss(dt_part);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            // date-only → TOD = 0
            return 0.0;
        }

        double tod = tm.tm_hour + tm.tm_min / 60.0 + tm.tm_sec / 3600.0;

        if (!has_explicit_tz) {
            // Naive/Z string: treat as UTC, apply study-site shift
            tod += tz_shift_hours;
        }
        // For explicit-TZ strings: tod is already local wall-clock time

        // Normalise to [0, 24)
        tod = std::fmod(tod, 24.0);
        if (tod < 0.0) tod += 24.0;
        return tod;
    }

    // -------------------------------------------------------------------------
    // Extract day-of-year [0, 365] from ISO 8601 string.
    // Uses the LOCAL calendar date:
    //   - offset-free / "Z" strings: UTC date from parsed tm
    //   - explicit-TZ strings: local date from the datetime part
    // No mktime() call required — doy_from_tm() computes directly.
    // -------------------------------------------------------------------------
    static double extractTOY(const std::string& iso_timestamp) {
        std::string dt_part;
        double tz_offset_hours = 0.0;
        parseTZOffset(iso_timestamp, dt_part, tz_offset_hours);

        std::tm tm = {};
        std::istringstream ss(dt_part);

        // Try full datetime first
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            // Try date-only
            ss.clear();
            ss.str(dt_part);
            ss >> std::get_time(&tm, "%Y-%m-%d");
            if (ss.fail()) {
                throw std::runtime_error("Invalid ISO timestamp format: " + iso_timestamp);
            }
        }

        return static_cast<double>(doy_from_tm(tm));
    }
};

#endif
