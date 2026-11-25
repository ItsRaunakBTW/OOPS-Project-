/**
 * @file PowerSubsystem.cpp
 * @brief Implementation of the PowerSubsystem class (ALF-EWG project).
 *
 * Key C++ concepts demonstrated here:
 *   1. Call by Reference – setValues() modifies the caller's variables.
 *   2. Friend Function   – auditPower() accesses private data directly.
 *   3. Boundary clamping – chargeBattery / consumePower keep the battery
 *      within [0, 100] without exposing the raw field.
 */

#include "PowerSubsystem.h"

#include <algorithm>  // std::min, std::max
#include <iostream>
#include <iomanip>    // std::fixed, std::setprecision

namespace DisasterCore {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
PowerSubsystem::PowerSubsystem(std::string id, float battery, float solar)
    : subsystemId(std::move(id))
    , batteryLevel(std::max(0.0f, std::min(100.0f, battery)))
    , solarInputWatts(std::max(0.0f, solar))
{}

// ---------------------------------------------------------------------------
// chargeBattery
// ---------------------------------------------------------------------------
void PowerSubsystem::chargeBattery(float watts) {
    if (watts < 0.0f) return;                          // ignore negative input
    batteryLevel = std::min(100.0f, batteryLevel + watts);
}

// ---------------------------------------------------------------------------
// consumePower
// ---------------------------------------------------------------------------
void PowerSubsystem::consumePower(float watts) {
    if (watts < 0.0f) return;                          // ignore negative draw
    batteryLevel = std::max(0.0f, batteryLevel - watts);
}

// ---------------------------------------------------------------------------
// setValues  –  CALL BY REFERENCE demonstration
// ---------------------------------------------------------------------------
/**
 * Both parameters arrive as references to the caller's local (or member)
 * variables.  Writing to them here writes directly to the caller's storage —
 * no copy is involved and the caller immediately sees the updated values once
 * this function returns.
 *
 * This is the idiomatic C++ pattern for returning multiple values without
 * resorting to pointers or structs.
 */
void PowerSubsystem::setValues(float& battery, float& solar) {
    // Clamp and store the new battery value.
    batteryLevel = std::max(0.0f, std::min(100.0f, battery));

    // Clamp and store the new solar input value.
    solarInputWatts = std::max(0.0f, solar);

    // Write the (clamped) accepted values back through the references so the
    // caller knows exactly what was stored (call-by-reference side-effect).
    battery = batteryLevel;
    solar   = solarInputWatts;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------
float PowerSubsystem::getBatteryLevel() const {
    return batteryLevel;
}

float PowerSubsystem::getSolarInput() const {
    return solarInputWatts;
}

std::string PowerSubsystem::getSubsystemId() const {
    return subsystemId;
}

// ---------------------------------------------------------------------------
// displayStatus
// ---------------------------------------------------------------------------
void PowerSubsystem::displayStatus() const {
    std::cout << std::fixed << std::setprecision(2)
              << "[PowerSubsystem " << subsystemId << "]\n"
              << "  Battery Level : " << batteryLevel    << " %\n"
              << "  Solar Input   : " << solarInputWatts << " W\n";
}

// ---------------------------------------------------------------------------
// auditPower  –  FRIEND FUNCTION definition (non-member)
// ---------------------------------------------------------------------------
/**
 * CONCEPT – Friend Function:
 *   This function is defined OUTSIDE the class body and is NOT called with
 *   object.auditPower() syntax — it is a free function.  However, because
 *   it was declared as a friend inside PowerSubsystem, the compiler allows
 *   it to reach into the private section of the object passed to it.
 *
 *   Here it reads ps.batteryLevel directly (private!) to produce a low-level
 *   audit trace that bypasses any formatting the public API might apply.
 *
 *   Friend relationships are one-way and non-transitive:  auditPower can see
 *   PowerSubsystem's private members, but PowerSubsystem cannot see the
 *   internals of whatever type auditPower itself might be part of.
 */
void auditPower(const PowerSubsystem& ps) {
    std::cout << "[AUDIT] Subsystem '" << ps.subsystemId
              << "' raw batteryLevel = " << ps.batteryLevel << "\n";
              // ^^^ Direct access to private member — allowed because
              //     auditPower is a friend of PowerSubsystem.
}

} // namespace DisasterCore
