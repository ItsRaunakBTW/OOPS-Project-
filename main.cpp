// =============================================================================
// main.cpp
// ALF-EWG – Autonomous Landslide & Flash-Flood Early Warning Grid
// B.Tech CSE OOP Project (C++17)
//
// This file drives the complete menu-driven console application.
// Every module from Unit I – Unit IV is exercised here.
// =============================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <stdexcept>

// Project headers
#include "SensorStation.h"      // Unit I & II: SensorStation, overloads, inline fns
#include "SensorNode.h"         // Unit III: inheritance hierarchy, polymorphism
#include "AlertSystem.h"        // Unit II & IV: EarlyWarningSiren, SensorCluster, functor
#include "IncidentManager.h"    // Unit IV: binary file I/O, exceptions
#include "Exceptions.h"         // Unit IV: custom exception classes
#include "PowerSubsystem.h"     // Unit I: friend function, call by reference

// Bring namespaces into scope for readability
using namespace DisasterCore::Sensors;
using namespace DisasterCore::Alerts;
using namespace DisasterCore;

// =============================================================================
// Global application state
// =============================================================================
std::vector<SensorStation>  gStations;      // All registered sensor stations
EarlyWarningSiren           gSiren;         // The global early warning system
IncidentManager             gIncidentMgr;   // Binary file incident manager
SensorDataManager           gSensorDataMgr; // Text file sensor data manager

// Polymorphic sensor node vector (BaseSensorNode pointers) for Unit III demo
std::vector<BaseSensorNode*> gSensorNodes;

// Map: region name → average hazard index (Unit IV STL)
std::map<std::string, float> gRegionHazardMap;

// =============================================================================
// Utility helpers
// =============================================================================

// clearScreen – simple cross-platform console clear
void clearScreen() {
    std::cout << "\n\n";
}

// pauseScreen – waits for the user to press Enter
void pauseScreen() {
    std::cout << "\n  Press Enter to continue...";
    std::string line;
    std::getline(std::cin, line);
}

// printMainBanner – prints the application title banner
void printMainBanner() {
    std::cout << "\n";
    std::cout << "  ========================================\n";
    std::cout << "   AUTONOMOUS LANDSLIDE & FLASH-FLOOD\n";
    std::cout << "         EARLY WARNING GRID\n";
    std::cout << "            ALF-EWG v1.0\n";
    std::cout << "  ========================================\n";
}

// getIntInput – reads an integer from stdin, handles bad input gracefully
int getIntInput(const std::string& prompt) {
    int val = 0;
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return 0; // EOF reached
        }
        if (line.empty()) continue;
        try {
            val = std::stoi(line);
            return val;
        } catch (...) {
            std::cout << "  [!] Invalid input. Please enter a valid number.\n";
        }
    }
}

// getFloatInput – reads a float from stdin, handles bad input gracefully
float getFloatInput(const std::string& prompt) {
    float val = 0.0f;
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return 0.0f; // EOF reached
        }
        if (line.empty()) continue;
        try {
            val = std::stof(line);
            return val;
        } catch (...) {
            std::cout << "  [!] Invalid input. Please enter a valid number.\n";
        }
    }
}

// findStation – returns a pointer to the station with the given ID, or nullptr
SensorStation* findStation(int id) {
    for (auto& s : gStations) {
        if (s.getStationId() == id) return &s;
    }
    return nullptr;
}

// computeAndStoreHazard – runs all three overloaded computeHazardIndex
// variants for a station and picks the highest value as the composite index.
// Also updates the region hazard map.
float computeAndStoreHazard(SensorStation& st) {
    // Overload 1: Landslide risk
    float lsRisk = computeHazardIndex(st.getRainfallRate(), st.getSlopeAngle());
    // Overload 2: Seismic/soil risk (SeismicTag disambiguates)
    float seismicRisk = computeHazardIndex(st.getSoilMoisture(),
                                            st.getVibrationFreq(), SeismicTag{});
    // Overload 3: Flash-flood risk
    float floodRisk = computeHazardIndex(st.getRiverWaterLevel(),
                                          st.getRainfallRate(),
                                          st.getSlopeAngle());

    // Composite hazard = max of all three (most dangerous threat wins)
    float composite = std::max({lsRisk, seismicRisk, floodRisk});

    // Return by reference — we write the computed value directly into the object
    st.getHazardIndexRef() = composite;

    // Update the regional map (STL std::map)
    gRegionHazardMap[st.getRegionName()] = composite;

    return composite;
}

// triggerAlertIfNeeded – generates a warning alert for HIGH or CRITICAL risk
// and saves an incident record to the binary file.
void triggerAlertIfNeeded(SensorStation& st) {
    float idx    = st.getHazardIndex();
    std::string lvl = classifyRisk(idx);

    if (lvl == "HIGH" || lvl == "CRITICAL") {
        // Determine dominant hazard type for the alert label
        float lsRisk    = computeHazardIndex(st.getRainfallRate(), st.getSlopeAngle());
        float floodRisk = computeHazardIndex(st.getRiverWaterLevel(),
                                              st.getRainfallRate(),
                                              st.getSlopeAngle());
        std::string hazType = (floodRisk >= lsRisk) ? "FLASH_FLOOD" : "LANDSLIDE";

        gSiren.generateAlert(st.getStationId(), st.getRegionName(),
                             hazType, idx, lvl);

        // Auto-save to incident binary file (Unit IV file handling)
        try {
            gIncidentMgr.saveIncident(st.getStationId(), st.getRegionName(),
                                      hazType, idx, lvl);
        } catch (const SensorFailureException& e) {
            std::cerr << "  [FILE ERROR] " << e.what() << "\n";
        }
    }
}

// =============================================================================
// MODULE A – Sensor Station Management
// =============================================================================

void loadSampleData() {
    std::cout << "\n  Loading predefined sample sensor stations...\n\n";

    // Station 1 – LOW risk
    gStations.emplace_back(101, "Himalayan Sector A",  31.10f, 77.17f,
                            20.0f, 15.0f, 30.0f, 2.0f, 1.5f);
    // Station 2 – MODERATE risk
    gStations.emplace_back(102, "Himalayan Sector B",  31.25f, 77.30f,
                            60.0f, 30.0f, 55.0f, 5.0f, 3.5f);
    // Station 3 – HIGH risk
    gStations.emplace_back(103, "Himalayan Sector C",  31.40f, 77.50f,
                            120.0f, 55.0f, 75.0f, 12.0f, 7.0f);
    // Station 4 – CRITICAL risk
    gStations.emplace_back(104, "River Valley Zone",   30.90f, 77.00f,
                            180.0f, 70.0f, 90.0f, 18.0f, 12.5f);

    // Compute hazard for each sample station
    for (auto& s : gStations) {
        computeAndStoreHazard(s);
    }

    std::cout << "  4 sample stations loaded successfully.\n";
    std::cout << "  Total active stations: " << SensorStation::getTotalStations() << "\n";
}

void addNewStation() {
    std::cout << "\n  --- Add New Sensor Station ---\n";
    std::cout << "  1. Standard Prompt Entry\n";
    std::cout << "  2. Stream Extraction (operator>>)\n";
    int mode = getIntInput("  Select entry mode: ");

    if (mode == 2) {
        std::cout << "\n  [Demonstrating operator>> Stream Extraction]\n";
        SensorStation s;
        std::cin >> s;
        if (findStation(s.getStationId()) != nullptr) {
            std::cout << "  [!] Station ID " << s.getStationId() << " already exists.\n";
            return;
        }
        gStations.push_back(std::move(s));
        computeAndStoreHazard(gStations.back());
        std::cout << "\n  Station " << gStations.back().getStationId()
                  << " registered via operator>> successfully.\n";
        std::cout << "  Total active stations: " << SensorStation::getTotalStations() << "\n";
        return;
    }

    int id = getIntInput("  Enter Station ID     : ");

    // Check for duplicate ID
    if (findStation(id) != nullptr) {
        std::cout << "  [!] Station ID " << id << " already exists.\n";
        return;
    }

    std::string region;
    std::cout << "  Enter Region Name    : ";
    std::getline(std::cin, region);

    float lat  = getFloatInput("  Enter Latitude       : ");
    float lon  = getFloatInput("  Enter Longitude      : ");
    float rain = getFloatInput("  Rainfall Rate (mm/hr): ");
    float slope= getFloatInput("  Slope Angle (deg)    : ");
    float soil = getFloatInput("  Soil Moisture (0-100): ");
    float vib  = getFloatInput("  Vibration Freq (Hz)  : ");
    float river= getFloatInput("  River Water Level (m): ");

    gStations.emplace_back(id, region, lat, lon, rain, slope, soil, vib, river);
    computeAndStoreHazard(gStations.back());

    std::cout << "\n  Station " << id << " registered successfully.\n";
    std::cout << "  Total active stations: " << SensorStation::getTotalStations() << "\n";
}

void displayAllStations() {
    if (gStations.empty()) {
        std::cout << "\n  No stations registered yet.\n";
        return;
    }
    std::cout << "\n  --- Registered Sensor Stations ---\n\n";
    // std::for_each from STL – iterates and prints each station
    std::for_each(gStations.begin(), gStations.end(), [](const SensorStation& s) {
        // Uses overloaded operator<<
        std::cout << s << "\n";
    });
    std::cout << "  Total stations: " << SensorStation::getTotalStations() << "\n";
}

void viewStationDetail() {
    int id = getIntInput("  Enter Station ID to view: ");
    SensorStation* st = findStation(id);
    if (!st) {
        std::cout << "  [!] Station " << id << " not found.\n";
        return;
    }
    st->display();

    // Demonstrate TelemetryAuditor (friend class) – accesses private members directly
    std::cout << "\n  [TelemetryAuditor – Friend Class Access]\n";
    TelemetryAuditor auditor;
    auditor.auditStation(*st);
}

void updateStationReadings() {
    int id = getIntInput("  Enter Station ID to update: ");
    SensorStation* st = findStation(id);
    if (!st) {
        std::cout << "  [!] Station " << id << " not found.\n";
        return;
    }

    std::cout << "  Enter new sensor readings:\n";
    float rain  = getFloatInput("  Rainfall Rate (mm/hr): ");
    float slope = getFloatInput("  Slope Angle (deg)    : ");
    float soil  = getFloatInput("  Soil Moisture (0-100): ");
    float vib   = getFloatInput("  Vibration Freq (Hz)  : ");
    float river = getFloatInput("  River Water Level (m): ");

    // updateReadings uses call by reference (Unit I concept)
    st->updateReadings(rain, slope, soil, vib, river);
    float newIdx = computeAndStoreHazard(*st);

    std::cout << "\n  Readings updated. New Hazard Index: "
              << std::fixed << std::setprecision(2) << newIdx
              << " (" << classifyRisk(newIdx) << ")\n";

    // Auto-trigger alert if threshold exceeded
    triggerAlertIfNeeded(*st);
}

void deleteStation() {
    int id = getIntInput("  Enter Station ID to delete: ");
    auto it = std::find_if(gStations.begin(), gStations.end(),
                           [id](const SensorStation& s) {
                               return s.getStationId() == id;
                           });
    if (it == gStations.end()) {
        std::cout << "  [!] Station " << id << " not found.\n";
        return;
    }
    gRegionHazardMap.erase(it->getRegionName());
    gStations.erase(it);
    std::cout << "  Station " << id << " deleted.\n";
    std::cout << "  Total active stations: " << SensorStation::getTotalStations() << "\n";
}

void demonstrateDeepCopy() {
    if (gStations.empty()) {
        std::cout << "  [!] No stations available. Load sample data first.\n";
        return;
    }
    std::cout << "\n  --- Deep Copy Constructor Demonstration ---\n";
    std::cout << "  Copying Station " << gStations[0].getStationId() << "...\n\n";

    // Deep copy constructor is called here
    SensorStation copy = gStations[0];

    std::cout << "  ORIGINAL:\n";
    gStations[0].display();

    // Modify the original's hazard via return-by-reference
    gStations[0].getHazardIndexRef() = 99.9f;

    std::cout << "\n  After modifying original hazardIndex to 99.9:\n";
    std::cout << "  Original hazardIndex = "
              << std::fixed << std::setprecision(2)
              << gStations[0].getHazardIndex() << "\n";
    std::cout << "  Copy's   hazardIndex = "
              << std::fixed << std::setprecision(2)
              << copy.getHazardIndex() << "\n";
    std::cout << "  --> Copy is INDEPENDENT (deep copy confirmed).\n";

    // Restore original
    computeAndStoreHazard(gStations[0]);
}

void sensorStationMenu() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n  ==============================\n";
        std::cout << "   SENSOR STATION MANAGEMENT\n";
        std::cout << "  ==============================\n";
        std::cout << "  1. Load Sample Data\n";
        std::cout << "  2. Add New Station\n";
        std::cout << "  3. Display All Stations\n";
        std::cout << "  4. View Station Detail (+ Audit)\n";
        std::cout << "  5. Update Sensor Readings\n";
        std::cout << "  6. Delete a Station\n";
        std::cout << "  7. Deep Copy Demonstration\n";
        std::cout << "  8. Save Sensor Stations to File (sensor_data.dat)\n";
        std::cout << "  9. Load Sensor Stations from File (sensor_data.dat)\n";
        std::cout << "  0. Back to Main Menu\n";
        choice = getIntInput("  Enter choice: ");
        switch (choice) {
            case 1: loadSampleData();       break;
            case 2: addNewStation();        break;
            case 3: displayAllStations();   break;
            case 4: viewStationDetail();    break;
            case 5: updateStationReadings();break;
            case 6: deleteStation();        break;
            case 7: demonstrateDeepCopy();  break;
            case 8: {
                std::vector<std::pair<std::string, float>> exportData;
                for (const auto& s : gStations) {
                    exportData.emplace_back(s.getRegionName(), s.getHazardIndex());
                }
                gSensorDataMgr.saveSensorStations(exportData, "sensor_data.dat");
                break;
            }
            case 9: {
                std::vector<std::pair<std::string, float>> importedData;
                gSensorDataMgr.loadSensorStations(importedData, "sensor_data.dat");
                std::cout << "\n  --- Imported Station Telemetry Records ---\n";
                for (const auto& p : importedData) {
                    std::cout << "  Region: " << std::left << std::setw(25) << p.first
                              << " | Hazard Index: " << std::fixed << std::setprecision(2)
                              << p.second << " [" << classifyRisk(p.second) << "]\n";
                }
                break;
            }
            case 0: break;
            default: std::cout << "  [!] Invalid choice.\n"; break;
        }
        if (choice != 0) pauseScreen();
    }
}

// =============================================================================
// MODULE B – Hazard Analysis
// =============================================================================

void performHazardAnalysis() {
    if (gStations.empty()) {
        std::cout << "\n  [!] No stations registered. Load sample data first.\n";
        return;
    }

    std::cout << "\n  --- Hazard Analysis for All Stations ---\n\n";
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(22) << "Region"
              << std::setw(10) << "LS Risk"
              << std::setw(12) << "Seismic"
              << std::setw(12) << "Flood"
              << std::setw(10) << "Composite"
              << std::setw(10) << "Level" << "\n";
    std::cout << std::string(82, '-') << "\n";

    for (auto& st : gStations) {
        float ls    = computeHazardIndex(st.getRainfallRate(), st.getSlopeAngle());
        float seism = computeHazardIndex(st.getSoilMoisture(),
                                          st.getVibrationFreq(), SeismicTag{});
        float flood = computeHazardIndex(st.getRiverWaterLevel(),
                                          st.getRainfallRate(),
                                          st.getSlopeAngle());
        float comp  = computeAndStoreHazard(st);

        std::cout << std::fixed << std::setprecision(1)
                  << std::left
                  << std::setw(6)  << st.getStationId()
                  << std::setw(22) << st.getRegionName()
                  << std::setw(10) << ls
                  << std::setw(12) << seism
                  << std::setw(12) << flood
                  << std::setw(10) << comp
                  << std::setw(10) << classifyRisk(comp) << "\n";

        // Trigger alert for HIGH/CRITICAL stations
        triggerAlertIfNeeded(st);
    }
    std::cout << "\n  Hazard analysis complete. Alerts generated for HIGH/CRITICAL stations.\n";
}

void hazardAnalysisMenu() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n  ==============================\n";
        std::cout << "   HAZARD ANALYSIS\n";
        std::cout << "  ==============================\n";
        std::cout << "  1. Analyse All Stations\n";
        std::cout << "  2. Analyse a Specific Station\n";
        std::cout << "  0. Back\n";
        choice = getIntInput("  Enter choice: ");
        switch (choice) {
            case 1:
                performHazardAnalysis();
                break;
            case 2: {
                int id = getIntInput("  Enter Station ID: ");
                SensorStation* st = findStation(id);
                if (!st) { std::cout << "  Station not found.\n"; break; }
                float comp = computeAndStoreHazard(*st);
                std::cout << "\n  Station " << id
                          << " (" << st->getRegionName() << ")\n";
                std::cout << "  Landslide Risk : "
                          << std::fixed << std::setprecision(2)
                          << computeHazardIndex(st->getRainfallRate(), st->getSlopeAngle())
                          << "\n";
                std::cout << "  Seismic Risk   : "
                          << computeHazardIndex(st->getSoilMoisture(),
                                                 st->getVibrationFreq(), SeismicTag{})
                          << "\n";
                std::cout << "  Flood Risk     : "
                          << computeHazardIndex(st->getRiverWaterLevel(),
                                                 st->getRainfallRate(),
                                                 st->getSlopeAngle())
                          << "\n";
                std::cout << "  Composite Index: " << comp
                          << " (" << classifyRisk(comp) << ")\n";
                triggerAlertIfNeeded(*st);
                break;
            }
            case 0: break;
            default: std::cout << "  [!] Invalid choice.\n"; break;
        }
        if (choice != 0) pauseScreen();
    }
}

// =============================================================================
// MODULE C – Early Warning and Alert System
// =============================================================================

void alertSystemMenu() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n  ==============================\n";
        std::cout << "   EARLY WARNING ALERT SYSTEM\n";
        std::cout << "  ==============================\n";
        std::cout << "  Global Alert Level: "
                  << EarlyWarningSiren::getGlobalAlertLevel() << "\n";
        std::cout << "  Active Alerts     : "
                  << gSiren.getActiveAlertCount() << "\n";
        std::cout << "\n";
        std::cout << "  1. Display All Active Alerts\n";
        std::cout << "  2. Acknowledge an Alert\n";
        std::cout << "  3. Reset All Alerts\n";
        std::cout << "  4. Manually Set Global Alert Level (0-3)\n";
        std::cout << "  0. Back\n";
        choice = getIntInput("  Enter choice: ");
        switch (choice) {
            case 1:
                gSiren.displayActiveAlerts();
                break;
            case 2: {
                int aid = getIntInput("  Enter Alert ID to acknowledge: ");
                gSiren.acknowledgeAlert(aid);
                break;
            }
            case 3:
                gSiren.resetAllAlerts();
                std::cout << "  All alerts reset.\n";
                break;
            case 4: {
                int lvl = getIntInput("  Enter level (0=Normal,1=Watch,2=Warning,3=Emergency): ");
                EarlyWarningSiren::setGlobalAlertLevel(lvl);
                std::cout << "  Global alert level set to " << lvl << ".\n";
                break;
            }
            case 0: break;
            default: std::cout << "  [!] Invalid choice.\n"; break;
        }
        if (choice != 0) pauseScreen();
    }
}

// =============================================================================
// MODULE D – Regional Risk Management
// =============================================================================

void regionalRiskMenu() {
    if (gStations.empty()) {
        std::cout << "\n  [!] No stations registered. Load sample data first.\n";
        pauseScreen();
        return;
    }

    int choice = -1;
    while (choice != 0) {
        std::cout << "\n  ==============================\n";
        std::cout << "   REGIONAL RISK MANAGEMENT\n";
        std::cout << "  ==============================\n";
        std::cout << "  1. View Regional Hazard Map\n";
        std::cout << "  2. Sort Regions by Hazard (Descending)\n";
        std::cout << "  3. Demonstrate Sensor Cluster Merge (operator+)\n";
        std::cout << "  4. Compare Two Clusters (operator>)\n";
        std::cout << "  0. Back\n";
        choice = getIntInput("  Enter choice: ");
        switch (choice) {
            case 1: {
                // Display STL map contents
                std::cout << "\n  --- Regional Hazard Map ---\n\n";
                std::cout << std::left
                          << std::setw(28) << "Region"
                          << std::setw(14) << "Hazard Index"
                          << "Risk Level\n";
                std::cout << std::string(55, '-') << "\n";
                for (const auto& kv : gRegionHazardMap) {
                    std::cout << std::left
                              << std::setw(28) << kv.first
                              << std::fixed << std::setprecision(2)
                              << std::setw(14) << kv.second
                              << classifyRisk(kv.second) << "\n";
                }
                break;
            }
            case 2: {
                // Build vector of clusters, one per region, then sort
                std::vector<SensorCluster> clusters;
                // Group stations by region
                std::map<std::string, SensorCluster> clusterMap;
                for (const auto& kv : gRegionHazardMap) {
                    clusterMap.emplace(kv.first, SensorCluster(kv.first));
                }
                for (const auto& st : gStations) {
                    clusterMap.at(st.getRegionName())
                              .addStation(st.getStationId(), st.getHazardIndex());
                }
                for (auto& kv : clusterMap) {
                    clusters.push_back(kv.second);
                }
                // Sort descending using EvacuationPriority functor
                std::sort(clusters.begin(), clusters.end(), EvacuationPriority{});
                std::cout << "\n  --- Regions Sorted by Hazard (Most Dangerous First) ---\n\n";
                for (size_t i = 0; i < clusters.size(); ++i) {
                    std::cout << "  #" << (i + 1) << " " << clusters[i];
                }
                break;
            }
            case 3: {
                // Demonstrate operator+ on SensorCluster
                SensorCluster c1("Sector-A");
                c1.addStation(101, 20.5f);
                c1.addStation(102, 45.0f);
                SensorCluster c2("Sector-B");
                c2.addStation(103, 68.0f);
                c2.addStation(104, 92.5f);

                std::cout << "\n  Cluster 1:\n" << c1;
                std::cout << "\n  Cluster 2:\n" << c2;

                SensorCluster merged = c1 + c2;
                std::cout << "\n  Merged (operator+):\n" << merged;
                break;
            }
            case 4: {
                // Demonstrate operator> on SensorCluster
                SensorCluster ca("RegionA");
                ca.addStation(101, 35.0f);
                SensorCluster cb("RegionB");
                cb.addStation(102, 78.0f);
                std::cout << "\n  RegionA avg hazard: "
                          << std::fixed << std::setprecision(2)
                          << ca.getAverageHazard() << "\n";
                std::cout << "  RegionB avg hazard: " << cb.getAverageHazard() << "\n";
                if (ca > cb) std::cout << "  RegionA is MORE dangerous (operator> = true).\n";
                else         std::cout << "  RegionB is MORE dangerous (operator> = false for A>B).\n";
                break;
            }
            case 0: break;
            default: std::cout << "  [!] Invalid choice.\n"; break;
        }
        if (choice != 0) pauseScreen();
    }
}

// =============================================================================
// MODULE E – Evacuation Priority List
// =============================================================================

void showEvacuationList() {
    if (gStations.empty()) {
        std::cout << "\n  [!] No stations registered.\n";
        return;
    }

    // Build clusters (one per region) for the evacuation list
    std::map<std::string, SensorCluster> clusterMap;
    for (const auto& kv : gRegionHazardMap) {
        clusterMap.emplace(kv.first, SensorCluster(kv.first));
    }
    for (const auto& st : gStations) {
        clusterMap.at(st.getRegionName())
                  .addStation(st.getStationId(), st.getHazardIndex());
    }
    std::vector<SensorCluster> clusters;
    for (auto& kv : clusterMap) {
        clusters.push_back(kv.second);
    }

    std::cout << "\n  --- EVACUATION PRIORITY LIST ---\n";
    displayEvacuationList(clusters);  // Uses EvacuationPriority functor + std::sort
}

// =============================================================================
// MODULE F – Incident History
// =============================================================================

void incidentHistoryMenu() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n  ==============================\n";
        std::cout << "   INCIDENT HISTORY\n";
        std::cout << "  ==============================\n";
        std::cout << "  Incidents on file: " << gIncidentMgr.getIncidentCount() << "\n\n";
        std::cout << "  1. Display All Incidents\n";
        std::cout << "  2. Clear All Incidents\n";
        std::cout << "  3. Demonstrate Custom Exceptions\n";
        std::cout << "  0. Back\n";
        choice = getIntInput("  Enter choice: ");
        switch (choice) {
            case 1:
                gIncidentMgr.displayAllIncidents();
                break;
            case 2:
                gIncidentMgr.clearAllIncidents();
                break;
            case 3:
                gIncidentMgr.demonstrateExceptions();
                break;
            case 0: break;
            default: std::cout << "  [!] Invalid choice.\n"; break;
        }
        if (choice != 0) pauseScreen();
    }
}

// =============================================================================
// MODULE G – Complete Sensor Network Sweep (Unit III polymorphism demo)
// =============================================================================

void setupPolymorphicNodes() {
    // Clear any existing nodes
    for (auto* n : gSensorNodes) delete n;
    gSensorNodes.clear();

    // Create concrete sensor objects and push into the base-class pointer vector.
    // The vptr of each object will route readTelemetry() / evaluateThreshold()
    // to the correct overridden implementation at runtime.
    auto* soil   = new SoilMoistureSensor(201, "Himalayan Sector A");
    auto* seismic= new SeismicVibrationSensor(202, "Himalayan Sector B");
    auto* tilt   = new AcousticTiltSensor(203, "Himalayan Sector C");
    auto* river  = new RiverLevelSensor(204, "River Valley Zone");

    // Set readings that demonstrate all threshold states
    soil   ->setReading(85.0f);  // ALERT (>80)
    seismic->setReading(18.0f);  // ALERT (>15 Hz)
    tilt   ->setReading(30.0f);  // OK    (<45)
    river  ->setReading(10.5f);  // ALERT (>8 m)

    gSensorNodes.push_back(soil);
    gSensorNodes.push_back(seismic);
    gSensorNodes.push_back(tilt);
    gSensorNodes.push_back(river);
}

void runNetworkSweep() {
    std::cout << "\n  --- Setting up Polymorphic Sensor Nodes ---\n";
    setupPolymorphicNodes();

    std::cout << "\n  --- Running Sensor Network Sweep ---\n";
    std::cout << "  (Demonstrates runtime polymorphism via BaseSensorNode*)\n\n";
    runSensorNetworkSweep(gSensorNodes);  // Defined in SensorNode.cpp

    std::cout << "\n  --- Diamond Inheritance Demonstration ---\n";
    demonstrateDiamondInheritance();      // Defined in SensorNode.cpp

    std::cout << "\n  --- PowerSubsystem Friend Function Demo ---\n";
    PowerSubsystem ps("PS-SWEEP-01", 75.0f, 12.5f);
    ps.displayStatus();
    auditPower(ps);  // Friend function: accesses private batteryLevel directly

    std::cout << "\n  --- Call by Reference Demo (PowerSubsystem::setValues) ---\n";
    float myBattery = 88.5f, mySolar = 24.0f;
    std::cout << "  Before setValues(): battery=" << myBattery
              << "%, solar=" << mySolar << "W\n";
    ps.setValues(myBattery, mySolar);  // Modifies ps and returns clamped values via references
    std::cout << "  After setValues(): battery=" << myBattery
              << "%, solar=" << mySolar << "W (synchronized via reference)\n";
}

// =============================================================================
// MODULE H – System Statistics
// =============================================================================

void showSystemStats() {
    std::cout << "\n  ==============================\n";
    std::cout << "   SYSTEM STATISTICS\n";
    std::cout << "  ==============================\n";
    std::cout << "  Total Stations Registered : "
              << SensorStation::getTotalStations() << "\n";
    std::cout << "  Active Alerts             : "
              << gSiren.getActiveAlertCount() << "\n";
    std::cout << "  Global Alert Level        : "
              << EarlyWarningSiren::getGlobalAlertLevel()
              << " (0=Normal, 1=Watch, 2=Warning, 3=Emergency)\n";
    std::cout << "  Incidents on File         : "
              << gIncidentMgr.getIncidentCount() << "\n";
    std::cout << "  Regions Monitored         : "
              << gRegionHazardMap.size() << "\n";

    if (!gStations.empty()) {
        // Find most dangerous station using operator>
        SensorStation* worst = &gStations[0];
        for (auto& s : gStations) {
            if (s > *worst) worst = &s;
        }
        std::cout << "  Most Dangerous Station    : "
                  << worst->getStationId()
                  << " (" << worst->getRegionName() << ") – "
                  << std::fixed << std::setprecision(2)
                  << worst->getHazardIndex()
                  << " [" << classifyRisk(worst->getHazardIndex()) << "]\n";
    }

    std::cout << "\n  --- STL map: Region Hazard Snapshot ---\n";
    for (const auto& kv : gRegionHazardMap) {
        std::cout << "    " << std::left << std::setw(28) << kv.first
                  << std::fixed << std::setprecision(2) << kv.second
                  << " [" << classifyRisk(kv.second) << "]\n";
    }
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main() {
    printMainBanner();
    std::cout << "\n  Initializing disaster monitoring system...\n";
    std::cout << "  Incident history file: incident_history.dat\n";
    std::cout << "  Existing incidents on file: "
              << gIncidentMgr.getIncidentCount() << "\n";

    int choice = -1;
    while (choice != 0) {
        printMainBanner();
        std::cout << "\n";
        std::cout << "  1. Sensor Station Management\n";
        std::cout << "  2. Enter / Update Sensor Readings\n";
        std::cout << "  3. Perform Hazard Analysis\n";
        std::cout << "  4. View Active Emergency Alerts\n";
        std::cout << "  5. Regional Risk Analysis\n";
        std::cout << "  6. Evacuation Priority List\n";
        std::cout << "  7. View Incident History\n";
        std::cout << "  8. Run Complete Sensor Network Sweep\n";
        std::cout << "  9. System Statistics\n";
        std::cout << "  0. Exit\n";
        std::cout << "\n  ========================================\n";
        choice = getIntInput("  Enter choice: ");

        switch (choice) {
            case 1:
                sensorStationMenu();
                break;
            case 2:
                updateStationReadings();
                pauseScreen();
                break;
            case 3:
                hazardAnalysisMenu();
                break;
            case 4:
                alertSystemMenu();
                break;
            case 5:
                regionalRiskMenu();
                break;
            case 6:
                showEvacuationList();
                pauseScreen();
                break;
            case 7:
                incidentHistoryMenu();
                break;
            case 8:
                runNetworkSweep();
                pauseScreen();
                break;
            case 9:
                showSystemStats();
                pauseScreen();
                break;
            case 0:
                std::cout << "\n  Shutting down ALF-EWG. Stay safe!\n\n";
                break;
            default:
                std::cout << "  [!] Invalid choice. Please try again.\n";
                pauseScreen();
                break;
        }
    }

    // Cleanup dynamically allocated polymorphic sensor nodes
    for (auto* n : gSensorNodes) delete n;
    gSensorNodes.clear();

    return 0;
}
