// =============================================================================
// Exceptions.h
// ALF-EWG (Autonomous Landslide & Flash-Flood Early Warning Grid)
// Namespace: DisasterCore
//
// UNIT IV – Custom Exception Handling
//
// Three custom exception classes are derived from std::exception.
// Each overrides what() to return a meaningful error message.
// An optional detail string is passed via the constructor and
// appended to the base message, giving the caller richer context.
//
// Concepts demonstrated:
//   - Inheritance from std::exception
//   - Overriding what() with a const noexcept function
//   - try / catch / throw (demonstrated in IncidentManager.cpp)
// =============================================================================

#ifndef ALF_EWG_EXCEPTIONS_H
#define ALF_EWG_EXCEPTIONS_H

#include <exception>
#include <string>

namespace DisasterCore {

// ---------------------------------------------------------------------------
// SensorFailureException
// Thrown when a sensor hardware malfunction is detected, e.g., the sensor
// returns readings outside the physically possible range, or a file write
// for telemetry fails.
// ---------------------------------------------------------------------------
class SensorFailureException : public std::exception {
public:
    // Constructor: accepts an optional detail string to append to the message.
    explicit SensorFailureException(const std::string& detail = "")
        : message_("Sensor hardware failure detected.")
    {
        if (!detail.empty()) {
            message_ += " Detail: " + detail;
        }
    }

    // Override what() — returns the full error message.
    // const noexcept matches the signature declared in std::exception.
    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};


// ---------------------------------------------------------------------------
// TelemetryChecksumMismatch
// Thrown when a received telemetry packet fails its checksum verification,
// indicating data corruption during transmission.
// ---------------------------------------------------------------------------
class TelemetryChecksumMismatch : public std::exception {
public:
    explicit TelemetryChecksumMismatch(const std::string& detail = "")
        : message_("Telemetry checksum mismatch. Data may be corrupted.")
    {
        if (!detail.empty()) {
            message_ += " Detail: " + detail;
        }
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};


// ---------------------------------------------------------------------------
// ThresholdOverflowException
// Thrown when a sensor reading exceeds the safe operating threshold defined
// for that sensor type (e.g., vibration > 20 Hz, river level > 15 m).
// ---------------------------------------------------------------------------
class ThresholdOverflowException : public std::exception {
public:
    explicit ThresholdOverflowException(const std::string& detail = "")
        : message_("Sensor reading exceeds safe threshold limits.")
    {
        if (!detail.empty()) {
            message_ += " Detail: " + detail;
        }
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

} // namespace DisasterCore

#endif // ALF_EWG_EXCEPTIONS_H
