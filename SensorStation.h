// =============================================================================
// SensorStation.h
// ALF-EWG (Automated Landslide & Flood Early Warning Grid)
// Namespace: DisasterCore::Sensors
//
// Concepts demonstrated:
//   - Classes and objects
//   - Static data members and static member functions
//   - Deep copy constructor (dynamic memory management)
//   - Operator overloading (<<, >>, >)
//   - Friend class and friend function
//   - Function overloading (computeHazardIndex)
//   - Inline functions
//   - Call by reference (updateReadings)
//   - Return by reference (getHazardIndexRef)
// =============================================================================

#ifndef SENSOR_STATION_H
#define SENSOR_STATION_H

#include <iostream>
#include <string>

namespace DisasterCore {
namespace Sensors {

// ---------------------------------------------------------------------------
// Forward declaration of TelemetryAuditor so SensorStation can declare it
// as a friend class before TelemetryAuditor is fully defined below.
// ---------------------------------------------------------------------------
class TelemetryAuditor;

// ===========================================================================
// CLASS: SensorStation
// Represents a physical environmental sensor node deployed in a disaster-
// prone region. Each station captures rainfall, slope, soil moisture,
// vibration, and river-level readings and computes a composite hazard index.
// ===========================================================================
class SensorStation {
private:
    // -----------------------------------------------------------------------
    // Instance data members
    // -----------------------------------------------------------------------
    int         stationId;        // Unique numeric ID for this station
    std::string regionName;       // Human-readable region name
    float       latitude;         // Geographic latitude
    float       longitude;        // Geographic longitude

    float rainfallRate;     // mm/hr  – current rainfall intensity
    float slopeAngle;       // degrees – terrain slope at sensor location
    float soilMoisture;     // 0–100% – volumetric soil water content
    float vibrationFreq;    // Hz     – ground vibration (seismic activity)
    float riverWaterLevel;  // metres – river stage height

    float hazardIndex;      // 0–100 composite risk score (computed)

    // -----------------------------------------------------------------------
    // Dynamic buffer – stores [rainfallRate, slopeAngle, soilMoisture,
    // riverWaterLevel] for quick array-style access.
    // Allocated with `new float[4]` in constructor; freed in destructor.
    // -----------------------------------------------------------------------
    float* readingsBuffer;

    // -----------------------------------------------------------------------
    // Static member – shared by ALL instances of SensorStation.
    // Tracks the total number of stations created across the entire program.
    // Defined (and initialised to 0) once in SensorStation.cpp.
    // -----------------------------------------------------------------------
    static int totalStations;

public:
    // -----------------------------------------------------------------------
    // Default Constructor
    // Initialises telemetry to zero / default state, allocates dynamic buffer,
    // and increments totalStations.
    // -----------------------------------------------------------------------
    SensorStation();

    // -----------------------------------------------------------------------
    // Parameterised Constructor
    // Allocates `readingsBuffer` with `new float[4]`.
    // Increments the static counter totalStations.
    // -----------------------------------------------------------------------
    SensorStation(int id,
                  const std::string& region,
                  float lat, float lon,
                  float rainfall, float slope,
                  float soil, float vibration,
                  float riverLevel);

    // -----------------------------------------------------------------------
    // Deep Copy Constructor
    // Creates a fully independent copy: allocates a *new* buffer and copies
    // each element individually.  A shallow copy would share the same pointer
    // and cause a double-free on destruction.
    // -----------------------------------------------------------------------
    SensorStation(const SensorStation& other);

    // -----------------------------------------------------------------------
    // Move Constructor (Rule of Five)
    // Transfers ownership of readingsBuffer without new allocation.
    // -----------------------------------------------------------------------
    SensorStation(SensorStation&& other) noexcept;

    // -----------------------------------------------------------------------
    // Copy Assignment Operator (Rule of Five)
    // Safely handles self-assignment, reallocates buffer if needed, and copies
    // all fields deeply.
    // -----------------------------------------------------------------------
    SensorStation& operator=(const SensorStation& other);

    // -----------------------------------------------------------------------
    // Move Assignment Operator (Rule of Five)
    // Transfers ownership of readingsBuffer without new allocation.
    // -----------------------------------------------------------------------
    SensorStation& operator=(SensorStation&& other) noexcept;

    // -----------------------------------------------------------------------
    // Destructor – releases dynamic memory with `delete[]`
    // Decrements totalStations so the count stays accurate.
    // -----------------------------------------------------------------------
    ~SensorStation();

    // -----------------------------------------------------------------------
    // Getters (all const – do not modify the object)
    // -----------------------------------------------------------------------
    int         getStationId()       const { return stationId;       }
    std::string getRegionName()      const { return regionName;      }
    float       getLatitude()        const { return latitude;        }
    float       getLongitude()       const { return longitude;       }
    float       getRainfallRate()    const { return rainfallRate;    }
    float       getSlopeAngle()      const { return slopeAngle;      }
    float       getSoilMoisture()    const { return soilMoisture;    }
    float       getVibrationFreq()   const { return vibrationFreq;   }
    float       getRiverWaterLevel() const { return riverWaterLevel; }
    float       getHazardIndex()     const { return hazardIndex;     }

    // -----------------------------------------------------------------------
    // updateReadings – Call by Reference
    // Parameters are non-const references: the function reads the current
    // sensor values *from* the caller's variables and updates internal state.
    // Using references avoids copying and allows the function to reflect
    // updated values back to the caller if needed.
    // -----------------------------------------------------------------------
    void updateReadings(float& rainfall, float& slope, float& soil,
                        float& vibration, float& river);

    // -----------------------------------------------------------------------
    // getHazardIndexRef – Return by Reference
    // Returns a direct reference to the internal hazardIndex member,
    // allowing the caller to both read and modify it in-place.
    // -----------------------------------------------------------------------
    float& getHazardIndexRef();

    // -----------------------------------------------------------------------
    // Static member function – can be called without an instance.
    // Accesses only the static member totalStations.
    // -----------------------------------------------------------------------
    static int getTotalStations();

    // -----------------------------------------------------------------------
    // display – prints a formatted summary to stdout
    // -----------------------------------------------------------------------
    void display() const;

    // -----------------------------------------------------------------------
    // inputFromUser – reads all fields from std::cin interactively
    // -----------------------------------------------------------------------
    void inputFromUser();

    // -----------------------------------------------------------------------
    // Operator Overloads
    // -----------------------------------------------------------------------

    // operator<< (stream insertion) – friend so it can access private members.
    // Prints the station in a human-readable format.
    friend std::ostream& operator<<(std::ostream& os, const SensorStation& s);

    // operator>> (stream extraction) – friend; reads fields from a stream.
    friend std::istream& operator>>(std::istream& is, SensorStation& s);

    // operator> – compares two stations by their hazardIndex (higher = more
    // dangerous).  Returns true if *this has a greater hazardIndex than other.
    bool operator>(const SensorStation& other) const;

    // -----------------------------------------------------------------------
    // Friend class declaration
    // TelemetryAuditor is granted direct access to ALL private members of
    // SensorStation without needing getters.
    // -----------------------------------------------------------------------
    friend class TelemetryAuditor;
};

// ===========================================================================
// computeHazardIndex – Function Overloading
// Three overloads for different hazard scenarios.  The compiler selects the
// correct version based on the number and types of arguments.
//
// Overloads 1 and 2 would otherwise share the same signature (float, float).
// A lightweight SeismicTag dispatch struct differentiates them at zero runtime
// cost — a standard C++ "tag dispatch" idiom.
// ===========================================================================

// Tag type used solely to disambiguate Overload 2 from Overload 1.
struct SeismicTag {};

// Overload 1: Landslide risk (rainfall mm/hr + slope degrees)
float computeHazardIndex(float rainfallRate, float slopeAngle);

// Overload 2: Seismic / soil saturation risk (soil % + vibration Hz)
// Pass SeismicTag{} as the third argument to select this overload.
float computeHazardIndex(float soilSaturation, float vibrationFrequency,
                         SeismicTag);

// Overload 3: Flash-flood risk (river level m + rainfall mm/hr + slope degrees)
float computeHazardIndex(float riverLevel, float rainfallRate, float slopeAngle);

// ===========================================================================
// Inline utility functions
// `inline` suggests the compiler replace the call site with the function body
// directly, avoiding function-call overhead for small, frequently used helpers.
// ===========================================================================

// classifyRisk – converts a numeric index into a human-readable risk category
inline std::string classifyRisk(float index) {
    if (index < 25.0f)       return "LOW";
    else if (index < 50.0f)  return "MODERATE";
    else if (index < 75.0f)  return "HIGH";
    else                     return "CRITICAL";
}

// normalizeValue – scales val to a 0–100 percentage of maxVal
inline float normalizeValue(float val, float maxVal) {
    return (val / maxVal) * 100.0f;
}

// ===========================================================================
// CLASS: TelemetryAuditor
// A utility class that inspects SensorStation internals directly.
// Because it is declared as a `friend class` inside SensorStation, it can
// access private members (stationId, rainfallRate, vibrationFreq,
// soilMoisture) without going through public getters.
// ===========================================================================
class TelemetryAuditor {
public:
    // auditStation – prints a detailed diagnostic report by directly
    // reading private members of the SensorStation object.
    void auditStation(const SensorStation& s);
};

} // namespace Sensors
} // namespace DisasterCore

#endif // SENSOR_STATION_H
