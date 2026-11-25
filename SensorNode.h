// =============================================================================
// SensorNode.h
// ALF-EWG (Automated Landslide & Flood Early Warning Grid)
// Unit III: Inheritance hierarchy, containership (composition), and polymorphism
//
// Key OOP concepts demonstrated here:
//   1. Abstract base class (pure virtual functions → vtable created)
//   2. Containership / composition (BaseSensorNode *has-a* PowerSubsystem)
//   3. Single-level concrete inheritance (Soil, Seismic, Acoustic, River sensors)
//   4. Diamond inheritance with virtual base class (DisasterTelemetryDevice)
//   5. Runtime polymorphism via base-class pointer + dynamic dispatch
// =============================================================================

#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <string>
#include <vector>
#include <iostream>

// Include the PowerSubsystem (composition member used in BaseSensorNode)
#include "PowerSubsystem.h"

// ============================================================
// Namespace: DisasterCore::Sensors
// All sensor-related classes live here.
// ============================================================
namespace DisasterCore {
namespace Sensors {

// ============================================================
// CLASS: BaseSensorNode  (Abstract Base Class)
//
// Every physical sensor node deployed in the field derives from
// this class.  Because it declares pure virtual functions, the
// compiler generates a vtable for BaseSensorNode.  Every
// *object* of a derived class carries a hidden vptr (virtual
// pointer) that points to the correct vtable row for its actual
// runtime type — this is what makes dynamic dispatch possible.
//
// CONTAINERSHIP (Composition) vs Inheritance:
//   PowerSubsystem is a *member variable* (has-a), not a base
//   class (is-a).  This models the real-world truth that a node
//   *uses* a power unit but is not a specialisation of one.
//
// vtable created for BaseSensorNode; each object has a hidden
// vptr pointing to it.
// ============================================================
class BaseSensorNode {
public:
    // ----------------------------------------------------------
    // Constructor
    // ----------------------------------------------------------
    BaseSensorNode(int id, std::string loc, std::string type);

    // ----------------------------------------------------------
    // Virtual destructor — mandatory for polymorphic base classes.
    // Without it, deleting a derived object via a base pointer
    // would only call the base destructor (undefined behaviour).
    // ----------------------------------------------------------
    virtual ~BaseSensorNode();

    // ----------------------------------------------------------
    // Pure virtual interface — derived classes MUST override both.
    // Their presence makes BaseSensorNode abstract; you cannot
    // instantiate it directly.
    // ----------------------------------------------------------
    virtual void readTelemetry()       = 0;
    virtual void evaluateThreshold()   = 0;

    // ----------------------------------------------------------
    // Virtual (non-pure) — provides a default but is overridable.
    // ----------------------------------------------------------
    virtual std::string getNodeInfo() const;

    // ----------------------------------------------------------
    // Non-virtual accessors
    // ----------------------------------------------------------
    int         getNodeId()    const;
    std::string getLocation()  const;

protected:
    // ---- node identity ----------------------------------------
    int         nodeId;
    std::string location;
    std::string sensorType;

    // ---- live reading -----------------------------------------
    float       currentReading;

    // ---- CONTAINERSHIP ----------------------------------------
    // BaseSensorNode *has-a* PowerSubsystem (composition).
    // The PowerSubsystem object is embedded directly inside every
    // sensor node — its lifetime is tied to the node's lifetime.
    // This is different from inheritance: a BaseSensorNode is NOT
    // a PowerSubsystem; it simply *uses* one.
    PowerSubsystem powerUnit;
};


// ============================================================
// CLASS: DisasterTelemetryDevice
//
// Root of the diamond hierarchy.  SolarPoweredNode and
// WirelessLoRaTransceiver both derive from it *virtually*,
// so that AutonomousHybridProbe ends up with exactly ONE
// DisasterTelemetryDevice sub-object even though it inherits
// through two paths.
// ============================================================
class DisasterTelemetryDevice {
public:
    DisasterTelemetryDevice(std::string serial, bool active = true);
    virtual ~DisasterTelemetryDevice();

    virtual std::string getDeviceInfo() const;

protected:
    std::string deviceSerial;
    bool        isActive;
};


// ============================================================
// CLASS: SolarPoweredNode
//
// Virtual inheritance from DisasterTelemetryDevice ensures that
// when AutonomousHybridProbe merges both inheritance paths, only
// one DisasterTelemetryDevice sub-object exists (no ambiguity,
// no duplicated data).
// ============================================================
class SolarPoweredNode : public virtual DisasterTelemetryDevice {
public:
    SolarPoweredNode(std::string serial,
                     float       wattage,
                     float       efficiency);
    virtual ~SolarPoweredNode();

    virtual std::string getSolarInfo() const;

protected:
    float panelWattage;
    float chargeEfficiency;
};


// ============================================================
// CLASS: WirelessLoRaTransceiver
//
// Also virtually inherits DisasterTelemetryDevice — same
// reasoning as SolarPoweredNode above.
// ============================================================
class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice {
public:
    WirelessLoRaTransceiver(std::string serial,
                             float       freqMHz,
                             int         sigStrength);
    virtual ~WirelessLoRaTransceiver();

    virtual std::string getLoRaInfo() const;

protected:
    float transmitFreqMHz;
    int   signalStrength;
};


// ============================================================
// CLASS: AutonomousHybridProbe
//
// Diamond hierarchy resolved with virtual inheritance:
//
//          DisasterTelemetryDevice          ← shared single copy
//          /                     \
//   SolarPoweredNode    WirelessLoRaTransceiver
//          \                     /
//          AutonomousHybridProbe
//
// Without "virtual" inheritance, AutonomousHybridProbe would
// contain TWO DisasterTelemetryDevice sub-objects — one from
// each path — causing ambiguity when accessing deviceSerial or
// calling getDeviceInfo().  The "virtual" keyword in the two
// intermediate classes tells the compiler to share a single
// sub-object, so getDeviceInfo() is unambiguous and data is
// consistent.
//
// BaseSensorNode is a SEPARATE (non-diamond) base; it does not
// participate in the virtual inheritance diamond at all.
// ============================================================
class AutonomousHybridProbe
        : public BaseSensorNode,
          public SolarPoweredNode,
          public WirelessLoRaTransceiver
{
public:
    AutonomousHybridProbe(int         nodeId,
                          std::string location,
                          std::string deviceSerial,
                          float       panelWattage,
                          float       chargeEfficiency,
                          float       freqMHz,
                          int         signalStrength);

    virtual ~AutonomousHybridProbe();

    // --- Overrides of pure virtuals from BaseSensorNode -------
    void readTelemetry()     override;
    void evaluateThreshold() override;

    // --- Utility setter ---------------------------------------
    void setReading(float r);
};


// ============================================================
// Concrete sensor classes — all extend BaseSensorNode.
// Each implements readTelemetry() and evaluateThreshold().
// ============================================================

// ------------------------------------------------------------
// SoilMoistureSensor: measures soil water content (0–100 %)
// ------------------------------------------------------------
class SoilMoistureSensor : public BaseSensorNode {
public:
    SoilMoistureSensor(int id, std::string loc);
    ~SoilMoistureSensor() override;

    void readTelemetry()     override;  // prints moisture %
    void evaluateThreshold() override;  // ALERT if > 80 %
    void setReading(float r);
};


// ------------------------------------------------------------
// SeismicVibrationSensor: measures ground vibration in Hz
// ------------------------------------------------------------
class SeismicVibrationSensor : public BaseSensorNode {
public:
    SeismicVibrationSensor(int id, std::string loc);
    ~SeismicVibrationSensor() override;

    void readTelemetry()     override;  // prints vibration Hz
    void evaluateThreshold() override;  // ALERT if > 15 Hz
    void setReading(float r);
};


// ------------------------------------------------------------
// AcousticTiltSensor: measures surface tilt angle (degrees)
// ------------------------------------------------------------
class AcousticTiltSensor : public BaseSensorNode {
public:
    AcousticTiltSensor(int id, std::string loc);
    ~AcousticTiltSensor() override;

    void readTelemetry()     override;  // prints tilt angle
    void evaluateThreshold() override;  // ALERT if > 45 deg
    void setReading(float r);
};


// ------------------------------------------------------------
// RiverLevelSensor: measures river water level in metres
// ------------------------------------------------------------
class RiverLevelSensor : public BaseSensorNode {
public:
    RiverLevelSensor(int id, std::string loc);
    ~RiverLevelSensor() override;

    void readTelemetry()     override;  // prints water level m
    void evaluateThreshold() override;  // ALERT if > 8 m
    void setReading(float r);
};


// ============================================================
// Free functions
// ============================================================

// runSensorNetworkSweep:
//   Demonstrates runtime polymorphism (dynamic dispatch).
//   The vector holds BaseSensorNode* pointers; at runtime the
//   vptr of each object routes the call to the correct override
//   without the caller knowing the concrete type.
void runSensorNetworkSweep(std::vector<BaseSensorNode*>& nodes);

// demonstrateDiamondInheritance:
//   Creates an AutonomousHybridProbe and calls getDeviceInfo()
//   exactly ONCE — the virtual base ensures there is only one
//   DisasterTelemetryDevice sub-object to call it on.
void demonstrateDiamondInheritance();

} // namespace Sensors
} // namespace DisasterCore

#endif // SENSOR_NODE_H
