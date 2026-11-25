/**
 * @file PowerSubsystem.h
 * @brief Declaration of the PowerSubsystem class for the ALF-EWG project.
 *
 * PowerSubsystem models a solar-powered battery unit aboard a remote
 * disaster-monitoring station.  It demonstrates:
 *   - Encapsulation (private data, public interface)
 *   - Call by reference  (setValues)
 *   - Friend functions   (auditPower)
 */

#ifndef ALF_EWG_POWER_SUBSYSTEM_H
#define ALF_EWG_POWER_SUBSYSTEM_H

#include <string>

namespace DisasterCore {

class PowerSubsystem {
public:
    // ------------------------------------------------------------------
    // Constructor
    // ------------------------------------------------------------------
    /**
     * @param id      Human-readable identifier for this subsystem (e.g. "PS-01").
     * @param battery Initial battery level as a percentage [0, 100].
     * @param solar   Initial solar panel input in watts.
     */
    PowerSubsystem(std::string id, float battery = 100.0f, float solar = 0.0f);

    // ------------------------------------------------------------------
    // Mutators
    // ------------------------------------------------------------------
    /**
     * Adds @p watts to the battery, capping the result at 100 %.
     * Simulates energy harvested from the solar panel.
     */
    void chargeBattery(float watts);

    /**
     * Subtracts @p watts from the battery, flooring the result at 0 %.
     * Simulates power consumed by onboard systems.
     */
    void consumePower(float watts);

    /**
     * Call-by-reference setter.
     *
     * CONCEPT – Call by Reference:
     *   The parameters are passed as references (float&), so this function
     *   modifies the caller's variables directly — no copy is made and the
     *   new values are visible to the caller after the call returns.
     *   This is the C++ mechanism equivalent to "pass-by-pointer" in C but
     *   with cleaner syntax and guaranteed non-null semantics.
     *
     * @param battery  Reference to the caller's battery variable; receives
     *                 the current battery level after the call.
     * @param solar    Reference to the caller's solar variable; receives the
     *                 current solar input after the call.
     */
    void setValues(float& battery, float& solar);

    // ------------------------------------------------------------------
    // Accessors
    // ------------------------------------------------------------------
    float       getBatteryLevel() const;
    float       getSolarInput()   const;
    std::string getSubsystemId()  const;

    /**
     * Prints a formatted status line showing battery % and solar input.
     */
    void displayStatus() const;

    // ------------------------------------------------------------------
    // Friend function declaration
    // ------------------------------------------------------------------
    /**
     * CONCEPT – Friend Function:
     *   A friend function is NOT a member of the class, yet it has access
     *   to the class's private (and protected) members.  Friendship is
     *   granted explicitly by the class and cannot be inherited or assumed.
     *   It is used sparingly — here for a trusted auditing routine that must
     *   read the raw internal state without going through the public API.
     *
     * auditPower prints the raw (unformatted) private battery level for
     * diagnostic/auditing purposes.  It is defined in PowerSubsystem.cpp,
     * outside the class body.
     */
    friend void auditPower(const PowerSubsystem& ps);

private:
    // ------------------------------------------------------------------
    // Data members
    // ------------------------------------------------------------------
    std::string subsystemId;    ///< Unique identifier for this power subsystem.
    float       batteryLevel;   ///< Stored energy, in percent [0.0 – 100.0].
    float       solarInputWatts;///< Current solar panel power delivery in watts.
};

// --------------------------------------------------------------------------
// Friend function (non-member, defined in PowerSubsystem.cpp)
// --------------------------------------------------------------------------
/**
 * Audit function that directly reads the private batteryLevel field of
 * @p ps and prints it to stdout.  Declared here so callers can include
 * this header and use the function without knowing its implementation.
 */
void auditPower(const PowerSubsystem& ps);

} // namespace DisasterCore

#endif // ALF_EWG_POWER_SUBSYSTEM_H
