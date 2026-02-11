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

class TimestampCalculator {
public:
    // Parse ISO 8601 timestamp (e.g., "2024-01-15T10:30:00" or "2024-01-15")
    static std::chrono::system_clock::time_point parseISO(const std::string& iso_timestamp) {
        std::tm tm = {};
        std::istringstream ss(iso_timestamp);

        // Try full ISO format first: YYYY-MM-DDTHH:MM:SS
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (!ss.fail()) {
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }

        // Try date-only format: YYYY-MM-DD
        ss.clear();
        ss.str(iso_timestamp);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (!ss.fail()) {
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        }

        throw std::runtime_error("Invalid ISO timestamp format: " + iso_timestamp +
                                 ". Expected format: YYYY-MM-DDTHH:MM:SS or YYYY-MM-DD");
    }

    // Calculate hours between two ISO timestamps
    // Returns positive if to_iso is after from_iso, negative otherwise
    static double hoursBetween(const std::string& from_iso, const std::string& to_iso) {
        auto from_time = parseISO(from_iso);
        auto to_time = parseISO(to_iso);
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(to_time - from_time);
        return static_cast<double>(duration.count()) / 3600.0;
    }

    // Calculate hours from ISO timestamp to now
    // Returns positive if timestamp is in the past, negative if in the future
    static double hoursFromNow(const std::string& iso_timestamp) {
        auto target_time = parseISO(iso_timestamp);
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - target_time);
        return static_cast<double>(duration.count()) / 3600.0;
    }

    // Calculate hours from a reference date to an ISO timestamp
    static double hoursTo(const std::string& reference_iso, const std::string& target_iso) {
        return hoursBetween(reference_iso, target_iso);
    }
};

#endif
