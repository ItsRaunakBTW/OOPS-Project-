// =============================================================================
// SensorNode.cpp
// ALF-EWG (Automated Landslide & Flood Early Warning Grid)
// Unit III: Inheritance hierarchy, containership, and polymorphism
//
// Implementation file — see SensorNode.h for full documentation of concepts.
// =============================================================================

#include "SensorNode.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>   // std::min / std::max (not strictly needed but useful)
#include <iomanip>     // std::fixed / std::setprecision

// Bring the namespace into scope for the whole translation unit.
using namespace DisasterCore::Sensors;

// =============================================================================
// BaseSensorNode — abstract base class implementation
// =============================================================================

// vtable created for BaseSensorNode; each object has a hidden vptr pointing to it.
//
// When the compiler sees a class with at least one virtual function it emits a
// vtable (a static array of function pointers).  Every object of that class (or
// any class derived from it) stores a hidden vptr as its very first member.
// At construction time the vptr is set to point to the vtable of the most-derived
// concrete class.  A virtual call like node->readTelemetry() is compiled as
//   (*node->vptr[index_of_readTelemetry])(node)
// — the CPU follows the pointer, looks up the slot, and jumps.  This is dynamic
// dispatch: the correct override is selected at runtime, not compile time.

BaseSensorNode::BaseSensorNode(int id, std::string loc, std::string type)
    : nodeId(id),
      location(std::move(loc)),
      sensorType(std::move(type)),
      currentReading(0.0f),
      // CONTAINERSHIP (composition):
      // PowerSubsystem is constructed here as an embedded member.
      // The node *owns* the power unit — their lifetimes are coupled.
      // The subsystem id is derived from the node id for traceability.
      powerUnit("PS-" + std::to_string(id), 100.0f, 15.0f)
{}

BaseSensorNode::~BaseSensorNode()
{
    // powerUnit is destroyed automatically when BaseSensorNode is destroyed
    // (member destructor called by compiler-generated sequence).
}

std::string BaseSensorNode::getNodeInfo() const
{
    return "[Node #" + std::to_string(nodeId) +
           " | Type: " + sensorType +
           " | Location: " + location +
           " | Battery: " + std::to_string(static_cast<int>(powerUnit.getBatteryLevel())) + "%" +
           "]";
}

int BaseSensorNode::getNodeId() const { return nodeId; }

std::string BaseSensorNode::getLocation() const { return location; }


// =============================================================================
// DisasterTelemetryDevice — root of diamond hierarchy
// =============================================================================

DisasterTelemetryDevice::DisasterTelemetryDevice(std::string serial, bool active)
    : deviceSerial(std::move(serial)), isActive(active)
{}

DisasterTelemetryDevice::~DisasterTelemetryDevice() {}

std::string DisasterTelemetryDevice::getDeviceInfo() const
{
    return "[Device Serial: " + deviceSerial +
           " | Active: " + (isActive ? "YES" : "NO") + "]";
}


// =============================================================================
// SolarPoweredNode
// =============================================================================

// Constructor explicitly calls DisasterTelemetryDevice ctor.
// In a diamond hierarchy this call is *only honoured when
// SolarPoweredNode is the most-derived class*.  When
// AutonomousHybridProbe is the most-derived class, the virtual
// base is initialised by AutonomousHybridProbe's own mem-init
// list instead (C++ rule: the most-derived class constructs the
// shared virtual base).
SolarPoweredNode::SolarPoweredNode(std::string serial,
                                    float       wattage,
                                    float       efficiency)
    : DisasterTelemetryDevice(serial),
      panelWattage(wattage),
      chargeEfficiency(efficiency)
{}

SolarPoweredNode::~SolarPoweredNode() {}

std::string SolarPoweredNode::getSolarInfo() const
{
    return "[Solar: " + std::to_string(panelWattage) + " W panel | " +
           "Efficiency: " + std::to_string(static_cast<int>(chargeEfficiency * 100.0f)) + "%" +
           "]";
}


// =============================================================================
// WirelessLoRaTransceiver
// =============================================================================

WirelessLoRaTransceiver::WirelessLoRaTransceiver(std::string serial,
                                                  float       freqMHz,
                                                  int         sigStrength)
    : DisasterTelemetryDevice(serial),
      transmitFreqMHz(freqMHz),
      signalStrength(sigStrength)
{}

WirelessLoRaTransceiver::~WirelessLoRaTransceiver() {}

std::string WirelessLoRaTransceiver::getLoRaInfo() const
{
    return "[LoRa: " + std::to_string(static_cast<int>(transmitFreqMHz)) + " MHz | " +
           "Signal: " + std::to_string(signalStrength) + " dBm]";
}


// =============================================================================
// AutonomousHybridProbe
//
// Diamond inheritance — virtual base class prevents duplication:
//
//   Without "virtual":
//     AutonomousHybridProbe would contain TWO copies of
//     DisasterTelemetryDevice — one inherited via SolarPoweredNode and
//     one via WirelessLoRaTransceiver.  Calls like getDeviceInfo() or
//     access to deviceSerial would be ambiguous (compiler error).
//
//   With "virtual" on SolarPoweredNode and WirelessLoRaTransceiver:
//     The compiler merges both paths into ONE shared DisasterTelemetryDevice
//     sub-object.  AutonomousHybridProbe's constructor is responsible for
//     initialising the virtual base directly (even though it appears two
//     levels up), because it is the most-derived class.
// =============================================================================

AutonomousHybridProbe::AutonomousHybridProbe(int         id,
                                              std::string loc,
                                              std::string serial,
                                              float       wattage,
                                              float       efficiency,
                                              float       freqMHz,
                                              int         sigStrength)
    // --- Most-derived class must initialise the virtual base directly ---
    : DisasterTelemetryDevice(serial, true),
      // --- Other non-virtual bases ---
      BaseSensorNode(id, loc, "AutonomousHybridProbe"),
      SolarPoweredNode(serial, wattage, efficiency),
      WirelessLoRaTransceiver(serial, freqMHz, sigStrength)
{}

AutonomousHybridProbe::~AutonomousHybridProbe() {}

void AutonomousHybridProbe::readTelemetry()
{
    std::cout << "\n=== AutonomousHybridProbe Telemetry ===\n";

    // BaseSensorNode::getNodeInfo() — uses powerUnit (composition member)
    std::cout << "Node    : " << getNodeInfo() << "\n";

    // SolarPoweredNode::getSolarInfo()
    std::cout << "Solar   : " << getSolarInfo() << "\n";

    // WirelessLoRaTransceiver::getLoRaInfo()
    std::cout << "LoRa    : " << getLoRaInfo() << "\n";

    // PowerSubsystem::displayStatus() via the composition member
    std::cout << "Power   : ";
    powerUnit.displayStatus();

    // Current reading
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Reading : " << currentReading << " (generic probe units)\n";
}

void AutonomousHybridProbe::evaluateThreshold()
{
    // Dynamic threshold: ALERT if reading exceeds 70 units
    if (currentReading > 70.0f) {
        std::cout << "[ALERT] AutonomousHybridProbe #" << nodeId
                  << " at '" << location
                  << "': reading " << currentReading
                  << " EXCEEDS threshold of 70!\n";
    } else {
        std::cout << "[OK] AutonomousHybridProbe #" << nodeId
                  << " at '" << location
                  << "': reading " << currentReading << " within safe limits.\n";
    }
}

void AutonomousHybridProbe::setReading(float r)
{
    currentReading = r;
}


// =============================================================================
// SoilMoistureSensor
// =============================================================================

SoilMoistureSensor::SoilMoistureSensor(int id, std::string loc)
    : BaseSensorNode(id, std::move(loc), "SoilMoistureSensor")
{}

SoilMoistureSensor::~SoilMoistureSensor() {}

void SoilMoistureSensor::readTelemetry()
{
    // Runtime dispatch lands here when the base pointer's vptr points to
    // SoilMoistureSensor's vtable slot for readTelemetry().
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[SoilMoistureSensor] " << getNodeInfo()
              << " | Soil Moisture: " << currentReading << " %\n";
}

void SoilMoistureSensor::evaluateThreshold()
{
    if (currentReading > 80.0f) {
        std::cout << "[ALERT] Node #" << nodeId
                  << " (" << location
                  << "): Soil moisture " << currentReading
                  << "% EXCEEDS safe limit of 80%! Landslide risk HIGH.\n";
    } else {
        std::cout << "[OK] Node #" << nodeId
                  << " (" << location
                  << "): Soil moisture " << currentReading << "% within limits.\n";
    }
}

void SoilMoistureSensor::setReading(float r) { currentReading = r; }


// =============================================================================
// SeismicVibrationSensor
// =============================================================================

SeismicVibrationSensor::SeismicVibrationSensor(int id, std::string loc)
    : BaseSensorNode(id, std::move(loc), "SeismicVibrationSensor")
{}

SeismicVibrationSensor::~SeismicVibrationSensor() {}

void SeismicVibrationSensor::readTelemetry()
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[SeismicVibrationSensor] " << getNodeInfo()
              << " | Vibration: " << currentReading << " Hz\n";
}

void SeismicVibrationSensor::evaluateThreshold()
{
    if (currentReading > 15.0f) {
        std::cout << "[ALERT] Node #" << nodeId
                  << " (" << location
                  << "): Vibration " << currentReading
                  << " Hz EXCEEDS safe limit of 15 Hz! Seismic activity detected.\n";
    } else {
        std::cout << "[OK] Node #" << nodeId
                  << " (" << location
                  << "): Vibration " << currentReading << " Hz — ground stable.\n";
    }
}

void SeismicVibrationSensor::setReading(float r) { currentReading = r; }


// =============================================================================
// AcousticTiltSensor
// =============================================================================

AcousticTiltSensor::AcousticTiltSensor(int id, std::string loc)
    : BaseSensorNode(id, std::move(loc), "AcousticTiltSensor")
{}

AcousticTiltSensor::~AcousticTiltSensor() {}

void AcousticTiltSensor::readTelemetry()
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[AcousticTiltSensor] " << getNodeInfo()
              << " | Tilt Angle: " << currentReading << " degrees\n";
}

void AcousticTiltSensor::evaluateThreshold()
{
    if (currentReading > 45.0f) {
        std::cout << "[ALERT] Node #" << nodeId
                  << " (" << location
                  << "): Tilt angle " << currentReading
                  << " deg EXCEEDS critical 45 deg! Structural instability risk.\n";
    } else {
        std::cout << "[OK] Node #" << nodeId
                  << " (" << location
                  << "): Tilt angle " << currentReading << " deg — structure stable.\n";
    }
}

void AcousticTiltSensor::setReading(float r) { currentReading = r; }


// =============================================================================
// RiverLevelSensor
// =============================================================================

RiverLevelSensor::RiverLevelSensor(int id, std::string loc)
    : BaseSensorNode(id, std::move(loc), "RiverLevelSensor")
{}

RiverLevelSensor::~RiverLevelSensor() {}

void RiverLevelSensor::readTelemetry()
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[RiverLevelSensor] " << getNodeInfo()
              << " | Water Level: " << currentReading << " m\n";
}

void RiverLevelSensor::evaluateThreshold()
{
    if (currentReading > 8.0f) {
        std::cout << "[ALERT] Node #" << nodeId
                  << " (" << location
                  << "): River level " << currentReading
                  << " m EXCEEDS flood threshold of 8 m! Evacuate downstream zones.\n";
    } else {
        std::cout << "[OK] Node #" << nodeId
                  << " (" << location
                  << "): River level " << currentReading << " m — within safe range.\n";
    }
}

void RiverLevelSensor::setReading(float r) { currentReading = r; }


// =============================================================================
// Free functions
// =============================================================================

// ----------------------------------------------------------------------------
// runSensorNetworkSweep
//
// Runtime polymorphism demonstration:
//   - The vector stores BaseSensorNode* (base-class pointers).
//   - The actual objects pointed to may be any concrete subclass.
//   - Calling node->readTelemetry() uses the vptr stored in each object to
//     look up the correct vtable entry at runtime — this is dynamic dispatch.
//   - The function does NOT need to know the concrete type; the vtable
//     mechanism routes each call correctly.
// ----------------------------------------------------------------------------
void DisasterCore::Sensors::runSensorNetworkSweep(std::vector<BaseSensorNode*>& nodes)
{
    std::cout << "\n========================================\n";
    std::cout << "   ALF-EWG Sensor Network Sweep Start  \n";
    std::cout << "========================================\n";

    for (BaseSensorNode* node : nodes) {
        if (node == nullptr) continue;

        // Dynamic dispatch via vptr:
        //   The compiler does NOT generate a direct call to SoilMoistureSensor
        //   ::readTelemetry or SeismicVibrationSensor::readTelemetry.
        //   Instead it emits:  CALL [node->vptr + offset_readTelemetry]
        //   At runtime the CPU follows node->vptr to the vtable, reads the
        //   function pointer from the correct slot, and jumps to it.
        node->readTelemetry();
        node->evaluateThreshold();
        std::cout << "----------------------------------------\n";
    }

    std::cout << "   ALF-EWG Sensor Network Sweep Done   \n";
    std::cout << "========================================\n";
}

// ----------------------------------------------------------------------------
// demonstrateDiamondInheritance
//
// Creates one AutonomousHybridProbe and calls getDeviceInfo() ONCE.
// Because SolarPoweredNode and WirelessLoRaTransceiver both use
// *virtual* inheritance from DisasterTelemetryDevice, the probe
// object contains exactly ONE DisasterTelemetryDevice sub-object.
// There is no ambiguity, and no duplicated deviceSerial field.
// ----------------------------------------------------------------------------
void DisasterCore::Sensors::demonstrateDiamondInheritance()
{
    std::cout << "\n============================================\n";
    std::cout << "   Diamond Inheritance Demonstration        \n";
    std::cout << "============================================\n";

    // Construct the probe: initialises the virtual base (DisasterTelemetryDevice)
    // exactly once, via AutonomousHybridProbe's own mem-init list.
    AutonomousHybridProbe probe(
        /*nodeId        =*/ 99,
        /*location      =*/ "Command HQ",
        /*deviceSerial  =*/ "ALF-PROBE-99",
        /*panelWattage  =*/ 50.0f,
        /*chargeEff     =*/ 0.82f,
        /*freqMHz       =*/ 868.0f,
        /*signalStrength=*/ -95
    );

    probe.setReading(85.5f);

    // getDeviceInfo() is unambiguous — only ONE DisasterTelemetryDevice
    // sub-object exists in 'probe', so there is exactly one getDeviceInfo()
    // to call.  No "request for member 'getDeviceInfo' is ambiguous" error.
    std::cout << "Device Info (called once — diamond resolved):\n";
    std::cout << "  " << probe.getDeviceInfo() << "\n";

    std::cout << "\nSolar Info:\n";
    std::cout << "  " << probe.getSolarInfo() << "\n";

    std::cout << "\nLoRa Info:\n";
    std::cout << "  " << probe.getLoRaInfo() << "\n";

    std::cout << "\nFull Telemetry Sweep:\n";
    probe.readTelemetry();
    probe.evaluateThreshold();

    std::cout << "============================================\n";
    std::cout << "   Diamond demo complete — no duplicate     \n";
    std::cout << "   DisasterTelemetryDevice sub-object.      \n";
    std::cout << "============================================\n\n";
}
