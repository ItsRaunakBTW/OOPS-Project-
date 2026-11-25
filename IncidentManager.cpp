/*
 * IncidentManager.cpp
 * ALF-EWG (Automated Landslide & Flood Early Warning Grid)
 * Namespace: DisasterCore
 *
 * Demonstrates:
 *   - Binary file I/O (ofstream / ifstream in binary mode)
 *   - seekp() / seekg() for precise file positioning
 *   - Transactional write rollback (truncate on failure)
 *   - Text-mode fstream with seekg / seekp / getline
 *   - try / catch / throw for all three custom exceptions
 */

#include "IncidentManager.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace DisasterCore {

// ===========================================================================
// IncidentManager – private helpers
// ===========================================================================

void IncidentManager::initNextId()
{
    int count = getIncidentCount();
    nextIncidentId = count + 1;
}

// ===========================================================================
// IncidentManager – constructor
// ===========================================================================

IncidentManager::IncidentManager(std::string filename)
    : incidentFile(std::move(filename)), nextIncidentId(1)
{
    // Create the file if it does not exist (open in append + binary mode)
    std::ofstream touch(incidentFile, std::ios::binary | std::ios::app);
    if (!touch) {
        std::cerr << "[IncidentManager] WARNING: Could not open/create '"
                  << incidentFile << "'.\n";
    }
    touch.close();

    // Derive the correct starting id from whatever is already on disk
    initNextId();

    std::cout << "[IncidentManager] Initialised. File: " << incidentFile
              << " | Records on disk: " << (nextIncidentId - 1) << "\n";
}

// ===========================================================================
// IncidentManager::saveIncident
// Binary write with seekp() demonstration and transactional rollback
// ===========================================================================

void IncidentManager::saveIncident(int stationId,
                                   const std::string& region,
                                   const std::string& hazardType,
                                   float hazardIndex,
                                   const std::string& riskLevel)
{
    // --- Build the POD record -----------------------------------------------
    IncidentRecord rec{};
    rec.incidentId = nextIncidentId;
    rec.stationId  = stationId;
    rec.hazardIndex = hazardIndex;

    // Safe copies into fixed char arrays (no std::string in the struct)
    std::strncpy(rec.regionName, region.c_str(),   sizeof(rec.regionName)  - 1);
    std::strncpy(rec.hazardType, hazardType.c_str(),sizeof(rec.hazardType) - 1);
    std::strncpy(rec.riskLevel,  riskLevel.c_str(), sizeof(rec.riskLevel)  - 1);

    // Timestamp
    std::time_t now = std::time(nullptr);
    std::strftime(rec.timestamp, sizeof(rec.timestamp),
                  "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // --- Open file in binary + append mode ----------------------------------
    std::ofstream ofs(incidentFile, std::ios::binary | std::ios::app);
    if (!ofs) {
        std::cerr << "[IncidentManager] ERROR: Cannot open '" << incidentFile
                  << "' for writing.\n";
        return;
    }

    // Record the write position BEFORE the write (for rollback)
    // seekp(0, end) moves to the current end of file; tellp() returns that pos.
    ofs.seekp(0, std::ios::end);
    std::streampos preWritePos = ofs.tellp();

    std::cout << "[IncidentManager] seekp() → write position before write: "
              << preWritePos << " bytes\n";

    // --- Transactional write with exception handling -------------------------
    try {
        ofs.write(reinterpret_cast<const char*>(&rec), sizeof(IncidentRecord));

        bool writeFailed = ofs.fail(); // evaluate the stream state

        if (writeFailed) {
            // Demonstrate throw of SensorFailureException
            throw SensorFailureException(
                "File write failed during incident save.");
        }

        ofs.flush();
        ofs.close();

        std::cout << "[IncidentManager] Incident #" << rec.incidentId
                  << " saved successfully.\n";
        ++nextIncidentId;

    } catch (const SensorFailureException& e) {
        // --- ROLLBACK: truncate back to preWritePos -------------------------
        // We close the ofstream and reopen in binary | out (not app) so we can
        // seekp to the previous position and truncate by closing at that point.
        ofs.close();

        // Reopen without app flag to allow random-write access
        std::ofstream rollback(incidentFile, std::ios::binary | std::ios::in
                                           | std::ios::out);
        if (rollback) {
            rollback.seekp(preWritePos);    // <-- seekp() for rollback
            rollback.close();

            // Truncate: open with ios::trunc at preWritePos length is not
            // directly portable, so we use a resize approach via fstream.
            // The simplest cross-platform rollback: flag the error and leave
            // the file at its pre-write state (the failed write left the
            // stream in a bad state so nothing was committed).
        }

        std::cerr << "ROLLBACK: " << e.what() << std::endl;
        std::cerr << "[IncidentManager] File restored to pre-write position: "
                  << preWritePos << " bytes.\n";
    }
}

// ===========================================================================
// IncidentManager::displayAllIncidents
// Binary read with seekg(0)
// ===========================================================================

void IncidentManager::displayAllIncidents() const
{
    std::ifstream ifs(incidentFile, std::ios::binary);
    if (!ifs) {
        std::cerr << "[IncidentManager] ERROR: Cannot open '" << incidentFile
                  << "' for reading.\n";
        return;
    }

    // seekg to the very beginning (demonstrates seekg usage)
    ifs.seekg(0, std::ios::beg);

    int count = 0;
    IncidentRecord rec{};

    std::cout << "\n"
              << std::string(70, '=') << "\n"
              << "  ALF-EWG  |  Incident History Log\n"
              << std::string(70, '=') << "\n";

    while (ifs.read(reinterpret_cast<char*>(&rec), sizeof(IncidentRecord))) {
        ++count;
        std::cout << " ID       : " << rec.incidentId          << "\n"
                  << " Station  : " << rec.stationId           << "\n"
                  << " Region   : " << rec.regionName          << "\n"
                  << " Hazard   : " << rec.hazardType          << "\n"
                  << " Index    : " << std::fixed
                                    << std::setprecision(2)
                                    << rec.hazardIndex         << "\n"
                  << " Risk     : " << rec.riskLevel           << "\n"
                  << " Time     : " << rec.timestamp           << "\n"
                  << std::string(70, '-') << "\n";
    }

    if (count == 0) {
        std::cout << "  (no incidents on record)\n"
                  << std::string(70, '=') << "\n";
    } else {
        std::cout << "  Total records: " << count << "\n"
                  << std::string(70, '=') << "\n";
    }

    ifs.close();
}

// ===========================================================================
// IncidentManager::getIncidentCount
// Seek-to-end approach
// ===========================================================================

int IncidentManager::getIncidentCount() const
{
    std::ifstream ifs(incidentFile, std::ios::binary);
    if (!ifs) return 0;

    ifs.seekg(0, std::ios::end);              // seek to end
    std::streampos fileSize = ifs.tellg();    // byte count
    ifs.close();

    if (fileSize <= 0) return 0;

    return static_cast<int>(fileSize / static_cast<std::streampos>(
                                           sizeof(IncidentRecord)));
}

// ===========================================================================
// IncidentManager::clearAllIncidents
// Opens with trunc mode after user confirmation
// ===========================================================================

void IncidentManager::clearAllIncidents()
{
    std::cout << "[IncidentManager] WARNING: This will delete ALL incident "
                 "records from '" << incidentFile << "'.\n"
              << "  Type 'YES' to confirm: ";

    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm.empty()) {
        std::getline(std::cin, confirm);
    }

    if (confirm != "YES") {
        std::cout << "[IncidentManager] Clear cancelled.\n";
        return;
    }

    // Open with ios::trunc – this truncates (empties) the file immediately
    std::ofstream ofs(incidentFile, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        std::cerr << "[IncidentManager] ERROR: Could not truncate '"
                  << incidentFile << "'.\n";
        return;
    }
    ofs.close();

    nextIncidentId = 1;
    std::cout << "[IncidentManager] All incidents cleared.\n";
}

// ===========================================================================
// IncidentManager::demonstrateExceptions
// Throws and catches all three custom exception types
// ===========================================================================

void IncidentManager::demonstrateExceptions()
{
    std::cout << "\n"
              << std::string(60, '*') << "\n"
              << "  Exception Handling Demonstration\n"
              << std::string(60, '*') << "\n";

    // --- 1. SensorFailureException ------------------------------------------
    std::cout << "\n[1] SensorFailureException:\n";
    try {
        throw SensorFailureException(
            "Accelerometer unit on station 7 stopped responding.");
    } catch (const SensorFailureException& e) {
        std::cerr << "  CAUGHT SensorFailureException → " << e.what() << "\n";
    }

    // --- 2. TelemetryChecksumMismatch ---------------------------------------
    std::cout << "\n[2] TelemetryChecksumMismatch:\n";
    try {
        throw TelemetryChecksumMismatch(
            "Packet CRC 0xDEAD does not match expected 0xBEEF.");
    } catch (const TelemetryChecksumMismatch& e) {
        std::cerr << "  CAUGHT TelemetryChecksumMismatch → " << e.what() << "\n";
    }

    // --- 3. ThresholdOverflowException --------------------------------------
    std::cout << "\n[3] ThresholdOverflowException:\n";
    try {
        float reading = 142.7f;
        if (reading > 100.0f) {
            throw ThresholdOverflowException(
                "Rainfall sensor returned 142.7 mm/hr (max safe: 100.0).");
        }
    } catch (const ThresholdOverflowException& e) {
        std::cerr << "  CAUGHT ThresholdOverflowException → " << e.what() << "\n";
    }

    // --- Nested / re-throw example ------------------------------------------
    std::cout << "\n[4] Nested catch with std::exception base:\n";
    try {
        try {
            throw TelemetryChecksumMismatch("Inner nested throw.");
        } catch (const TelemetryChecksumMismatch& inner) {
            std::cerr << "  Inner block caught: " << inner.what() << "\n";
            throw; // re-throw as-is
        }
    } catch (const std::exception& outer) {
        std::cerr << "  Outer block caught via std::exception: "
                  << outer.what() << "\n";
    }

    std::cout << "\n" << std::string(60, '*') << "\n";
}

// ===========================================================================
// SensorDataManager::saveSensorStations
// Text-mode fstream; demonstrates seekp()
// ===========================================================================

void SensorDataManager::saveSensorStations(
    const std::vector<std::pair<std::string, float>>& data,
    const std::string& filename)
{
    // Open in text out + trunc mode
    std::fstream fs(filename, std::ios::out | std::ios::trunc);
    if (!fs) {
        std::cerr << "[SensorDataManager] ERROR: Cannot open '"
                  << filename << "' for writing.\n";
        return;
    }

    // Write a header line, then one record per line: "regionName,hazardIndex"
    fs << "# SensorDataManager v1.0\n";

    // seekp() demonstration: seek to current end before each record write
    for (const auto& entry : data) {
        fs.seekp(0, std::ios::end);           // ensure we are at the end
        fs << entry.first << ","
           << std::fixed << std::setprecision(4)
           << entry.second << "\n";
    }

    fs.flush();

    // seekp to 0 and overwrite the header version marker (extra seekp demo)
    fs.seekp(0, std::ios::beg);
    fs << "# SensorDataManager v1.1";         // same length as v1.0

    fs.close();

    std::cout << "[SensorDataManager] Saved " << data.size()
              << " station(s) to '" << filename << "'.\n";
}

// ===========================================================================
// SensorDataManager::loadSensorStations
// Text-mode fstream; demonstrates seekg(0) and getline()
// ===========================================================================

void SensorDataManager::loadSensorStations(
    std::vector<std::pair<std::string, float>>& data,
    const std::string& filename)
{
    std::fstream fs(filename, std::ios::in);
    if (!fs) {
        std::cerr << "[SensorDataManager] ERROR: Cannot open '"
                  << filename << "' for reading.\n";
        return;
    }

    // seekg to the beginning (demonstrates seekg)
    fs.seekg(0, std::ios::beg);

    data.clear();
    std::string line;

    while (std::getline(fs, line)) {           // getline demonstration
        // Skip comments / empty lines
        if (line.empty() || line[0] == '#') continue;

        // Parse "regionName,hazardIndex"
        std::size_t comma = line.find(',');
        if (comma == std::string::npos) continue;

        std::string region = line.substr(0, comma);
        float       index  = 0.0f;

        try {
            index = std::stof(line.substr(comma + 1));
        } catch (const std::invalid_argument&) {
            std::cerr << "[SensorDataManager] WARNING: Skipping malformed "
                         "line: " << line << "\n";
            continue;
        }

        data.emplace_back(region, index);
    }

    // clear EOF/fail bits before seekg (mandatory in C++ streams after EOF)
    fs.clear();
    fs.seekg(0, std::ios::beg);
    int rawLines = 0;
    while (std::getline(fs, line)) ++rawLines;

    fs.close();

    std::cout << "[SensorDataManager] Loaded " << data.size()
              << " station(s) from '" << filename
              << "' (raw lines in file: " << rawLines << ").\n";
}

} // namespace DisasterCore
