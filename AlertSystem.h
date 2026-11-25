#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

/*
 * AlertSystem.h
 * ALF-EWG (Automated Landslide & Flash-flood Early Warning Grid)
 *
 * Demonstrates:
 *  - Namespaces (DisasterCore::Alerts)
 *  - Custom stream manipulators
 *  - Structs with initialization
 *  - Static class members
 *  - Operator overloading (+, >, <<)
 *  - Functor (EvacuationPriority)
 *  - STL vector usage
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

namespace DisasterCore {
namespace Alerts {

// ============================================================
// Custom stream manipulators
// ============================================================

/**
 * alertHeader – prints a formatted separator line.
 * Usage: std::cout << alertHeader;
 */
std::ostream& alertHeader(std::ostream& os);

/**
 * alertFooter – prints a formatted separator line (same style).
 * Usage: std::cout << alertFooter;
 */
std::ostream& alertFooter(std::ostream& os);

// ============================================================
// Alert struct
// ============================================================

struct Alert {
    int         alertId;
    int         stationId;
    std::string regionName;
    std::string hazardType;   // "LANDSLIDE" or "FLASH_FLOOD"
    float       hazardIndex;
    std::string riskLevel;    // LOW / MODERATE / HIGH / CRITICAL
    bool        acknowledged;
    std::string timestamp;    // set at creation time using ctime
};

// ============================================================
// EarlyWarningSiren class
// ============================================================

class EarlyWarningSiren {
private:
    // Shared across all instances
    static int globalAlertLevel; // 0=normal, 1=watch, 2=warning, 3=emergency
    static int nextAlertId;

    std::vector<Alert> activeAlerts;

public:
    // ---- Static accessors ----
    static int  getGlobalAlertLevel();
    static void setGlobalAlertLevel(int level);

    // ---- Instance methods ----

    /**
     * generateAlert – creates an Alert, pushes it to activeAlerts,
     * and simulates siren output on the console.
     */
    void generateAlert(int stationId,
                       const std::string& region,
                       const std::string& hazardType,
                       float hazardIndex,
                       const std::string& riskLevel);

    /**
     * displayActiveAlerts – prints all active alerts using the
     * custom alertHeader / alertFooter manipulators.
     */
    void displayActiveAlerts() const;

    /**
     * acknowledgeAlert – marks the alert with the given ID as acknowledged.
     */
    void acknowledgeAlert(int alertId);

    /**
     * resetAllAlerts – clears activeAlerts and resets globalAlertLevel to 0.
     */
    void resetAllAlerts();

    /**
     * getActiveAlertCount – returns the number of alerts in activeAlerts.
     */
    int getActiveAlertCount() const;
};

// ============================================================
// SensorCluster class
// ============================================================

class SensorCluster {
private:
    std::string      clusterName;
    float            totalHazardIndex;
    int              stationCount;
    std::vector<int> stationIds;

public:
    // Constructor
    explicit SensorCluster(std::string name);

    // Accessors
    float       getAverageHazard()  const;
    std::string getClusterName()    const;
    int         getStationCount()   const;

    /**
     * addStation – registers a station and accumulates its hazard index.
     */
    void addStation(int stationId, float hazardIndex);

    // ---- Operator overloads ----

    /**
     * operator+ – merges two clusters into a new one named "A+B".
     * Combines stationIds and sums totalHazardIndex.
     */
    SensorCluster operator+(const SensorCluster& other) const;

    /**
     * operator> – compares by average hazard index.
     */
    bool operator>(const SensorCluster& other) const;

    /**
     * operator<< – formatted output using alertHeader manipulator.
     */
    friend std::ostream& operator<<(std::ostream& os, const SensorCluster& sc);
};

// ============================================================
// Evacuation priority functor
// ============================================================

/**
 * EvacuationPriority – comparator functor for sorting SensorClusters.
 * Returns true when 'a' has a HIGHER average hazard than 'b',
 * yielding a descending-order sort.
 */
struct EvacuationPriority {
    bool operator()(const SensorCluster& a, const SensorCluster& b) const;
};

/**
 * displayEvacuationList – sorts clusters in descending hazard order
 * using EvacuationPriority, then prints each one.
 */
void displayEvacuationList(std::vector<SensorCluster>& clusters);

} // namespace Alerts
} // namespace DisasterCore

#endif // ALERTSYSTEM_H
