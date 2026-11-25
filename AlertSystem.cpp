/*
 * AlertSystem.cpp
 * ALF-EWG (Automated Landslide & Flash-flood Early Warning Grid)
 *
 * Implementation of:
 *  - Custom stream manipulators: alertHeader, alertFooter
 *  - EarlyWarningSiren class (static members, alert management)
 *  - SensorCluster class (operator overloading, functor)
 *  - displayEvacuationList free function
 */

#include "AlertSystem.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace DisasterCore {
namespace Alerts {

// ============================================================
// Static member definitions
// ============================================================

int EarlyWarningSiren::globalAlertLevel = 0;
int EarlyWarningSiren::nextAlertId      = 1;

// ============================================================
// Custom stream manipulators
// ============================================================

std::ostream& alertHeader(std::ostream& os) {
    os << "========================================\n";
    return os;
}

std::ostream& alertFooter(std::ostream& os) {
    os << "========================================\n";
    return os;
}

// ============================================================
// EarlyWarningSiren – static accessors
// ============================================================

int EarlyWarningSiren::getGlobalAlertLevel() {
    return globalAlertLevel;
}

void EarlyWarningSiren::setGlobalAlertLevel(int level) {
    if (level >= 0 && level <= 3) {
        globalAlertLevel = level;
    } else {
        std::cerr << "[EarlyWarningSiren] Invalid alert level: " << level
                  << ". Must be 0-3.\n";
    }
}

// ============================================================
// EarlyWarningSiren – generateAlert
// ============================================================

void EarlyWarningSiren::generateAlert(int stationId,
                                      const std::string& region,
                                      const std::string& hazardType,
                                      float hazardIndex,
                                      const std::string& riskLevel)
{
    // Build timestamp string using ctime
    std::time_t now = std::time(nullptr);
    std::string ts  = std::ctime(&now);
    // ctime appends '\n'; strip it
    if (!ts.empty() && ts.back() == '\n') {
        ts.pop_back();
    }

    // Construct the alert
    Alert alert;
    alert.alertId      = nextAlertId++;
    alert.stationId    = stationId;
    alert.regionName   = region;
    alert.hazardType   = hazardType;
    alert.hazardIndex  = hazardIndex;
    alert.riskLevel    = riskLevel;
    alert.acknowledged = false;
    alert.timestamp    = ts;

    activeAlerts.push_back(alert);

    // Automatically elevate the global alert level based on risk
    if (riskLevel == "CRITICAL" && globalAlertLevel < 3) {
        globalAlertLevel = 3;
    } else if (riskLevel == "HIGH" && globalAlertLevel < 2) {
        globalAlertLevel = 2;
    } else if (riskLevel == "MODERATE" && globalAlertLevel < 1) {
        globalAlertLevel = 1;
    }

    // Simulate siren output
    std::cout << alertHeader;
    std::cout << "  *** SIREN ACTIVATED ***\n";
    std::cout << "  Alert ID   : " << alert.alertId     << "\n";
    std::cout << "  Station    : " << stationId          << "\n";
    std::cout << "  Region     : " << region             << "\n";
    std::cout << "  Hazard     : " << hazardType         << "\n";
    std::cout << "  Index      : " << std::fixed << std::setprecision(2)
              << hazardIndex << "\n";
    std::cout << "  Risk Level : " << riskLevel          << "\n";
    std::cout << "  Time       : " << ts                 << "\n";
    std::cout << "  Global Lvl : " << globalAlertLevel   << " (";
    switch (globalAlertLevel) {
        case 0: std::cout << "NORMAL";    break;
        case 1: std::cout << "WATCH";     break;
        case 2: std::cout << "WARNING";   break;
        case 3: std::cout << "EMERGENCY"; break;
        default: std::cout << "UNKNOWN";  break;
    }
    std::cout << ")\n";
    std::cout << alertFooter;
}

// ============================================================
// EarlyWarningSiren – displayActiveAlerts
// ============================================================

void EarlyWarningSiren::displayActiveAlerts() const {
    std::cout << alertHeader;
    std::cout << "  ACTIVE ALERTS  [Total: " << activeAlerts.size() << "]\n";
    std::cout << alertFooter;

    if (activeAlerts.empty()) {
        std::cout << "  No active alerts.\n";
        std::cout << alertFooter;
        return;
    }

    for (const Alert& a : activeAlerts) {
        std::cout << "  ID        : " << a.alertId      << "\n";
        std::cout << "  Station   : " << a.stationId    << "\n";
        std::cout << "  Region    : " << a.regionName   << "\n";
        std::cout << "  Hazard    : " << a.hazardType   << "\n";
        std::cout << "  Index     : " << std::fixed << std::setprecision(2)
                  << a.hazardIndex << "\n";
        std::cout << "  Risk      : " << a.riskLevel    << "\n";
        std::cout << "  Time      : " << a.timestamp    << "\n";
        std::cout << "  Ack'd     : " << (a.acknowledged ? "Yes" : "No") << "\n";
        std::cout << alertFooter;
    }
}

// ============================================================
// EarlyWarningSiren – acknowledgeAlert
// ============================================================

void EarlyWarningSiren::acknowledgeAlert(int alertId) {
    for (Alert& a : activeAlerts) {
        if (a.alertId == alertId) {
            a.acknowledged = true;
            std::cout << "[AlertSystem] Alert #" << alertId << " acknowledged.\n";
            return;
        }
    }
    std::cout << "[AlertSystem] Alert #" << alertId << " not found.\n";
}

// ============================================================
// EarlyWarningSiren – resetAllAlerts
// ============================================================

void EarlyWarningSiren::resetAllAlerts() {
    activeAlerts.clear();
    globalAlertLevel = 0;
    std::cout << "[AlertSystem] All alerts cleared. Global level reset to NORMAL.\n";
}

// ============================================================
// EarlyWarningSiren – getActiveAlertCount
// ============================================================

int EarlyWarningSiren::getActiveAlertCount() const {
    return static_cast<int>(activeAlerts.size());
}

// ============================================================
// SensorCluster – constructor
// ============================================================

SensorCluster::SensorCluster(std::string name)
    : clusterName(std::move(name)),
      totalHazardIndex(0.0f),
      stationCount(0)
{}

// ============================================================
// SensorCluster – accessors
// ============================================================

float SensorCluster::getAverageHazard() const {
    if (stationCount == 0) return 0.0f;
    return totalHazardIndex / static_cast<float>(stationCount);
}

std::string SensorCluster::getClusterName() const {
    return clusterName;
}

int SensorCluster::getStationCount() const {
    return stationCount;
}

// ============================================================
// SensorCluster – addStation
// ============================================================

void SensorCluster::addStation(int stationId, float hazardIndex) {
    stationIds.push_back(stationId);
    totalHazardIndex += hazardIndex;
    ++stationCount;
}

// ============================================================
// SensorCluster – operator+
// ============================================================

SensorCluster SensorCluster::operator+(const SensorCluster& other) const {
    SensorCluster merged(clusterName + "+" + other.clusterName);

    // Copy own stations
    for (int id : stationIds) {
        merged.stationIds.push_back(id);
    }
    // Append other's stations
    for (int id : other.stationIds) {
        merged.stationIds.push_back(id);
    }

    merged.totalHazardIndex = totalHazardIndex + other.totalHazardIndex;
    merged.stationCount     = stationCount + other.stationCount;

    return merged;
}

// ============================================================
// SensorCluster – operator>
// ============================================================

bool SensorCluster::operator>(const SensorCluster& other) const {
    return getAverageHazard() > other.getAverageHazard();
}

// ============================================================
// SensorCluster – operator<<
// ============================================================

std::ostream& operator<<(std::ostream& os, const SensorCluster& sc) {
    os << alertHeader;
    os << "  Cluster    : " << sc.clusterName    << "\n";
    os << "  Stations   : " << sc.stationCount   << "\n";
    os << "  Total Idx  : " << std::fixed << std::setprecision(2)
       << sc.totalHazardIndex << "\n";
    os << "  Avg Hazard : " << std::fixed << std::setprecision(2)
       << sc.getAverageHazard() << "\n";
    os << "  Station IDs: ";
    for (std::size_t i = 0; i < sc.stationIds.size(); ++i) {
        if (i > 0) os << ", ";
        os << sc.stationIds[i];
    }
    os << "\n";
    os << alertFooter;
    return os;
}

// ============================================================
// EvacuationPriority functor
// ============================================================

bool EvacuationPriority::operator()(const SensorCluster& a,
                                    const SensorCluster& b) const {
    // Descending order: higher hazard comes first
    return a.getAverageHazard() > b.getAverageHazard();
}

// ============================================================
// displayEvacuationList
// ============================================================

void displayEvacuationList(std::vector<SensorCluster>& clusters) {
    // Sort descending by average hazard using the functor
    std::sort(clusters.begin(), clusters.end(), EvacuationPriority());

    std::cout << alertHeader;
    std::cout << "  EVACUATION PRIORITY LIST\n";
    std::cout << alertFooter;

    int priority = 1;
    for (const SensorCluster& sc : clusters) {
        std::cout << "  Priority #" << priority++ << "\n";
        std::cout << sc;
    }
}

} // namespace Alerts
} // namespace DisasterCore
