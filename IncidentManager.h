#ifndef INCIDENTMANAGER_H
#define INCIDENTMANAGER_H

/*
 * IncidentManager.h
 * ALF-EWG (Automated Landslide & Flood Early Warning Grid)
 * Namespace: DisasterCore
 *
 * Demonstrates:
 *   - Binary file I/O with ofstream / ifstream (binary mode)
 *   - seekp() / seekg() for file positioning
 *   - Transactional rollback on write failure
 *   - Text-mode fstream with seekg / seekp / getline
 *   - Custom exception handling (SensorFailureException, TelemetryChecksumMismatch,
 *     ThresholdOverflowException)
 */

#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <iostream>
#include <string>
#include <utility>   // std::pair
#include "Exceptions.h"

namespace DisasterCore {

// ---------------------------------------------------------------------------
// POD struct – binary-serialisable (no std::string, no virtual table)
// ---------------------------------------------------------------------------
struct IncidentRecord {
    int   incidentId;
    int   stationId;
    char  regionName[64];
    char  hazardType[32];
    float hazardIndex;
    char  riskLevel[16];
    char  timestamp[32];
};

// ---------------------------------------------------------------------------
// IncidentManager – manages a binary flat-file of IncidentRecord entries
// ---------------------------------------------------------------------------
class IncidentManager {
private:
    std::string incidentFile;   // default: "incident_history.dat"
    int         nextIncidentId;

    // Helper: derive the next id from the current record count
    void initNextId();

public:
    // Constructor – opens/creates the file and initialises nextIncidentId
    explicit IncidentManager(std::string filename = "incident_history.dat");

    /*
     * saveIncident
     * Appends a new IncidentRecord to the binary file.
     * - Uses ofstream in binary | app mode.
     * - Calls seekp() to verify/record the write position.
     * - Implements a basic transactional rollback: records the file size
     *   before the write; on failure it truncates back to that position and
     *   re-throws via SensorFailureException.
     */
    void saveIncident(int stationId,
                      const std::string& region,
                      const std::string& hazardType,
                      float hazardIndex,
                      const std::string& riskLevel);

    /*
     * displayAllIncidents
     * Opens the file with ifstream (binary), seekg(0) to the beginning,
     * reads records one-by-one and prints formatted output.
     */
    void displayAllIncidents() const;

    /*
     * getIncidentCount
     * Opens the file, seeks to the end, divides the byte offset by
     * sizeof(IncidentRecord) to calculate the record count.
     */
    int getIncidentCount() const;

    /*
     * clearAllIncidents
     * Asks for user confirmation then truncates the file by opening it
     * with ofstream in trunc mode.
     */
    void clearAllIncidents();

    /*
     * demonstrateExceptions
     * Shows try / catch / throw for all three custom exception types.
     */
    void demonstrateExceptions();
};

// ---------------------------------------------------------------------------
// SensorDataManager – text-mode fstream for key/value sensor station data
// ---------------------------------------------------------------------------
class SensorDataManager {
public:
    /*
     * saveSensorStations
     * Writes pairs of (regionName, hazardIndex) to a text file.
     * Uses fstream in out | trunc mode; demonstrates seekp().
     */
    void saveSensorStations(const std::vector<std::pair<std::string, float>>& data,
                            const std::string& filename = "sensor_data.dat");

    /*
     * loadSensorStations
     * Reads the same text file back into a vector of pairs.
     * Uses fstream in in mode; demonstrates seekg(0) and getline().
     */
    void loadSensorStations(std::vector<std::pair<std::string, float>>& data,
                            const std::string& filename = "sensor_data.dat");
};

} // namespace DisasterCore

#endif // INCIDENTMANAGER_H
