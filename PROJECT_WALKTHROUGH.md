# Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG)
## Comprehensive Code Walkthrough & Academic Concept Guide (Unit I – IV)

**Author / Project**: B.Tech CSE Object-Oriented Programming in C++  
**Standard**: Standard C++17 (Strict `-Wall -Wextra -Wpedantic -Werror`)  
**Project Path**: `/Users/raunzz/oops project/ALF-EWG/`

---

## Table of Contents
1. [Project Overview & System Architecture](#1-project-overview--system-architecture)
2. [End-to-End Simulation Flow](#2-end-to-end-simulation-flow)
3. [UNIT I: Core Foundations & Call Semantics](#3-unit-i-core-foundations--call-semantics)
   - Namespaces (`DisasterCore::Sensors`, `DisasterCore::Alerts`)
   - Inline Functions (`classifyRisk`, `normalizeValue`)
   - Function Overloading (`computeHazardIndex` with Tag Dispatch)
   - Call by Reference (`updateReadings`, `setValues`)
   - Return by Reference (`getHazardIndexRef`)
   - Friend Function (`auditPower`) and Friend Class (`TelemetryAuditor`)
4. [UNIT II: Encapsulation, Dynamic Memory & Operator Overloading](#4-unit-ii-encapsulation-dynamic-memory--operator-overloading)
   - Encapsulation & Const Member Functions
   - Static Members & Global Network State
   - Dynamic Memory Management (`new[]` / `delete[]`)
   - The Rule of Five (Deep Copy, Move Semantics, Destructors)
   - Operator Overloading (`+`, `>`, `<<`, `>>`)
5. [UNIT III: Inheritance, Containership & Polymorphism](#5-unit-iii-inheritance-containership--polymorphism)
   - Abstract Base Class (`BaseSensorNode`)
   - Containership (Composition) vs. Inheritance
   - Diamond Inheritance & Virtual Base Class Resolution
   - Runtime Polymorphism, vtables & Dynamic Dispatch
6. [UNIT IV: Streams, Persistence, Exception Handling & STL](#6-unit-iv-streams-persistence-exception-handling--stl)
   - Custom Exception Hierarchy (`std::exception` derivatives)
   - Binary File Serialization & Plain-Old-Data (POD) Layout
   - File Stream Positioning (`seekp`, `seekg`, `tellp`, `tellg`) & Transactional Rollback
   - Stream State Resetting (`fs.clear()`)
   - STL Containers, Algorithms & Custom Functor (`EvacuationPriority`)
   - Custom Stream Manipulators (`alertHeader`, `alertFooter`)
7. [Unit-Wise Syllabus Concept Mapping Table](#7-unit-wise-syllabus-concept-mapping-table)
8. [Viva Voce & Practical Examination Defense Guide](#8-viva-voce--practical-examination-defense-guide)

---

## 1. Project Overview & System Architecture

Mountainous terrains such as the Himalayas face frequent compound hazards: monsoonal cloudbursts, saturated soil profiles, seismic micro-tremors, and surging river levels. 

The **Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG)** is a modular, high-reliability simulation system written strictly in standard C++17. It manages physical and hybrid sensor probes, computes composite hazard ratings, activates sirens, maps regional risks, produces evacuation roadmaps, and saves mission-critical incident logs to non-volatile binary storage.

```
                    ┌─────────────────────────┐
                    │ DisasterTelemetryDevice │ (Root Class)
                    └────────────┬────────────┘
                                 │
                 ┌───────────────┴───────────────┐
                 │ virtual                       │ virtual
                 ▼                               ▼
      ┌──────────────────┐           ┌─────────────────────────┐
      │ SolarPoweredNode │           │ WirelessLoRaTransceiver │
      └──────────┬───────┘           └───────────┬─────────────┘
                 │                               │
                 └───────────────┬───────────────┘
                                 │ (Multiple Inheritance)
                                 ▼
                     ┌───────────────────────┐
                     │ AutonomousHybridProbe │
                     └───────────┬───────────┘
                                 │ public
                                 ▼
                    ┌─────────────────────────┐
                    │     BaseSensorNode      │ (Abstract Base Class)
                    │  [has-a PowerSubsystem] │ (Containership / Composition)
                    └────────────┬────────────┘
                                 │
     ┌───────────────────┬───────┴───────────┬───────────────────┐
     ▼                   ▼                   ▼                   ▼
┌──────────────┐ ┌────────────────┐ ┌──────────────────┐ ┌────────────────┐
│ SoilMoisture │ │ SeismicVibrate │ │ AcousticTiltNode │ │ RiverLevelNode │
│    Sensor    │ │     Sensor     │ │      Sensor      │ │     Sensor     │
└──────────────┘ └────────────────┘ └──────────────────┘ └────────────────┘
```

---

## 2. End-to-End Simulation Flow

1. **Station Management (`SensorStation`)**: Deployed field stations register their coordinates, current rainfall, slope, soil moisture, seismic frequency, and river stage height.
2. **Field Sensing Layer (`BaseSensorNode*`)**: Diverse physical probes collect real-time data under a unified polymorphic interface.
3. **Multi-Hazard Risk Engine (`computeHazardIndex`)**: Evaluates distinct risk models (landslide, seismic, flash-flood) and calculates a normalized 0–100 composite index.
4. **Early Warning Siren (`EarlyWarningSiren`)**: Stations with `HIGH` ($\ge 50$) or `CRITICAL` ($\ge 75$) hazard automatically trigger regional alarms and update the global network state (`NORMAL`, `WATCH`, `WARNING`, `EMERGENCY`).
5. **Regional Aggregation (`SensorCluster`)**: Groups stations by valley/sector. Overloaded operators (`+` and `>`) combine clusters and compare severity.
6. **Evacuation Routing (`EvacuationPriority`)**: Custom STL functors prioritize evacuation corridors based on risk.
7. **Incident Logging (`IncidentManager`)**: Binary file streams (`ios::binary`) write raw incident structs to `incident_history.dat` with pre-write rollback verification.

---

## 3. UNIT I: Core Foundations & Call Semantics

### A. Namespaces
* **Concept**: Namespaces partition the global identifier space to prevent name collisions.
* **Code Reference**: `SensorStation.h` & `AlertSystem.h`
* **Implementation**:
  ```cpp
  namespace DisasterCore {
      namespace Sensors {
          class SensorStation;
          class BaseSensorNode;
      }
      namespace Alerts {
          class EarlyWarningSiren;
          class SensorCluster;
      }
  }
  ```
* **Explanation**: Sensor hardware constructs are isolated in `DisasterCore::Sensors`, while alerting and siren logic resides in `DisasterCore::Alerts`.

---

### B. Inline Functions
* **Concept**: Advises the compiler to replace the function call with the function body, eliminating the call stack overhead (pushing registers, jumping, popping frames).
* **Code Reference**: `SensorStation.h`
* **Implementation**:
  ```cpp
  inline std::string classifyRisk(float index) {
      if (index < 25.0f)       return "LOW";
      else if (index < 50.0f)  return "MODERATE";
      else if (index < 75.0f)  return "HIGH";
      else                     return "CRITICAL";
  }

  inline float normalizeValue(float val, float maxVal) {
      return (val / maxVal) * 100.0f;
  }
  ```
* **Explanation**: `classifyRisk` is invoked frequently across console displays, file exports, and alert triggers; inlining optimizes execution speed.

---

### C. Function Overloading
* **Concept**: Functions sharing the same name but possessing unique parameter types or counts, resolved at compile-time via compiler name mangling.
* **Code Reference**: `SensorStation.h` & `SensorStation.cpp`
* **Implementation**:
  ```cpp
  // Overload 1: Landslide Hazard (Rainfall Rate + Terrain Slope)
  float computeHazardIndex(float rainfallRate, float slopeAngle);

  // Overload 2: Seismic Hazard (Soil Moisture + Vibration Frequency)
  // Uses a zero-cost tag struct to disambiguate identical (float, float) signatures
  struct SeismicTag {};
  float computeHazardIndex(float soilSaturation, float vibrationFrequency, SeismicTag);

  // Overload 3: Flash-Flood Hazard (River Level + Rainfall + Slope)
  float computeHazardIndex(float riverLevel, float rainfallRate, float slopeAngle);
  ```
* **Formulas Used**:
  $$\text{Landslide} = \left(\frac{\text{rain}}{200} \times 50\right) + \left(\frac{\text{slope}}{90} \times 50\right)$$
  $$\text{Seismic} = \left(\frac{\text{soil}}{100} \times 60\right) + \left(\frac{\text{vib}}{20} \times 40\right)$$
  $$\text{FlashFlood} = \left(\frac{\text{river}}{15} \times 50\right) + \left(\frac{\text{rain}}{200} \times 30\right) + \left(\frac{\text{slope}}{90} \times 20\right)$$

---

### D. Call by Reference
* **Concept**: Passing the memory address of an argument rather than copying its value, allowing the function to modify the caller's variable in-place without memory overhead.
* **Code Reference**: `SensorStation.cpp` (`updateReadings`) & `PowerSubsystem.cpp` (`setValues`)
* **Implementation**:
  ```cpp
  void SensorStation::updateReadings(float& rainfall, float& slope, float& soil,
                                     float& vibration, float& river) {
      rainfallRate    = rainfall;
      slopeAngle      = slope;
      soilMoisture    = soil;
      vibrationFreq   = vibration;
      riverWaterLevel = river;
      // Updates internal buffer...
  }
  ```

---

### E. Return by Reference
* **Concept**: Returning an lvalue reference (`float&`) to an internal member, allowing the caller to read and assign to the private member directly.
* **Code Reference**: `SensorStation.h` & `SensorStation.cpp`
* **Implementation**:
  ```cpp
  float& SensorStation::getHazardIndexRef() {
      return hazardIndex; // Returns an assignable lvalue reference
  }
  ```
* **Demonstration**:
  ```cpp
  // Mutates internal member directly:
  station.getHazardIndexRef() = 99.9f;
  ```

---

### F. Friend Function & Friend Class
* **Concept**: Bypasses traditional OOP access boundaries (`private` / `protected`) when explicitly granted friendship by the host class.
* **1. Friend Function**:
  - **Code Reference**: `PowerSubsystem.h` & `PowerSubsystem.cpp`
  - Declared inside `PowerSubsystem` as `friend void auditPower(const PowerSubsystem& ps);`.
  - Defined outside the class as a free function. Directly reads `ps.batteryLevel` without public getters.
* **2. Friend Class**:
  - **Code Reference**: `SensorStation.h`
  - Declared inside `SensorStation` as `friend class TelemetryAuditor;`.
  - `TelemetryAuditor::auditStation(const SensorStation& s)` directly reads private fields (`stationId`, `rainfallRate`, `vibrationFreq`, `soilMoisture`).

---

## 4. UNIT II: Encapsulation, Dynamic Memory & Operator Overloading

### A. Encapsulation & Const Member Functions
* **Concept**: Bundling data and functions while hiding internal state. Methods that do not modify member variables are marked `const`.
* **Code Reference**: `SensorStation.h`
  ```cpp
  int getStationId() const { return stationId; }
  float getHazardIndex() const { return hazardIndex; }
  ```

---

### B. Static Members & Network State
* **Concept**: Variables and functions tied to the class rather than instances, shared across all objects.
* **Code Reference**: `SensorStation.cpp` & `AlertSystem.cpp`
* **Implementation**:
  ```cpp
  // Defined once in .cpp:
  int SensorStation::totalStations = 0;

  int SensorStation::getTotalStations() {
      return totalStations;
  }
  ```
  Every constructor increments `totalStations`; destructors decrement it.

---

### C. Dynamic Memory & The Rule of Five
* **Concept**: When a class allocates raw heap memory (`new[]`), it must implement the **Rule of Five**: Destructor, Copy Constructor, Copy Assignment, Move Constructor, and Move Assignment.
* **Code Reference**: `SensorStation.cpp`

```cpp
// 1. Parameterized Constructor (Allocation)
SensorStation::SensorStation(...) {
    readingsBuffer = new float[4];
    ++totalStations;
}

// 2. Deep Copy Constructor
SensorStation::SensorStation(const SensorStation& other)
    : stationId(other.stationId), ... {
    readingsBuffer = new float[4]; // Fresh allocation
    for (int i = 0; i < 4; ++i) readingsBuffer[i] = other.readingsBuffer[i];
    ++totalStations;
}

// 3. Copy Assignment Operator (operator=)
SensorStation& SensorStation::operator=(const SensorStation& other) {
    if (this != &other) { // Self-assignment check
        if (!readingsBuffer) readingsBuffer = new float[4];
        for (int i = 0; i < 4; ++i) readingsBuffer[i] = other.readingsBuffer[i];
    }
    return *this;
}

// 4. Move Constructor (noexcept)
SensorStation::SensorStation(SensorStation&& other) noexcept
    : readingsBuffer(other.readingsBuffer) {
    other.readingsBuffer = nullptr; // Take ownership
    ++totalStations;
}

// 5. Move Assignment Operator (noexcept)
SensorStation& SensorStation::operator=(SensorStation&& other) noexcept {
    if (this != &other) {
        delete[] readingsBuffer;
        readingsBuffer = other.readingsBuffer;
        other.readingsBuffer = nullptr;
    }
    return *this;
}

// 6. Destructor (Deallocation)
SensorStation::~SensorStation() {
    delete[] readingsBuffer;
    readingsBuffer = nullptr;
    --totalStations;
}
```

---

### D. Operator Overloading
* **Binary `+`**: Merges two `SensorCluster` objects, concatenating station ID lists and summing hazard indices.
* **Relational `>`**: Compares two stations or clusters based on risk severity (`hazardIndex > other.hazardIndex`).
* **Stream Insertion `<<`**: Formats and prints station records cleanly (`std::cout << station`).
* **Stream Extraction `>>`**: Reads telemetry fields interactively (`std::cin >> station`).

---

## 5. UNIT III: Inheritance, Containership & Polymorphism

### A. Abstract Base Class (ABC)
* **Concept**: A blueprint containing pure virtual functions (`= 0`) that derived classes must override. Cannot be instantiated.
* **Code Reference**: `SensorNode.h`
* **Implementation**:
  ```cpp
  class BaseSensorNode {
  public:
      virtual ~BaseSensorNode();            // Virtual destructor
      virtual void readTelemetry()     = 0; // Pure virtual
      virtual void evaluateThreshold() = 0; // Pure virtual
  protected:
      int nodeId;
      std::string location;
      PowerSubsystem powerUnit;            // Containership
  };
  ```

---

### B. Containership (Composition) vs. Inheritance
* **Inheritance ("is-a")**: `SoilMoistureSensor` *is-a* `BaseSensorNode`.
* **Containership ("has-a")**: `BaseSensorNode` *has-a* `PowerSubsystem`. The solar battery is an embedded member whose lifecycle is tied directly to the node.

---

### C. Diamond Inheritance & Virtual Base Classes
* **Problem**: In multiple inheritance, inheriting through two paths causes duplicate copies of the root class, creating member ambiguity and memory bloat.
* **Solution**: `virtual` inheritance ensures only one root instance exists.
* **Code Reference**: `SensorNode.h`

```
              DisasterTelemetryDevice [Root]
              /                            \
   virtual   /                              \   virtual
            ▼                                ▼
   SolarPoweredNode              WirelessLoRaTransceiver
            \                                /
             \                              /
              ▼                            ▼
            AutonomousHybridProbe [Derived]
```

```cpp
class DisasterTelemetryDevice { ... };

// Intermediate classes inherit virtually:
class SolarPoweredNode : public virtual DisasterTelemetryDevice { ... };
class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice { ... };

// Leaf class inherits both without duplicate root sub-objects:
class AutonomousHybridProbe : public BaseSensorNode,
                              public SolarPoweredNode,
                              public WirelessLoRaTransceiver {
public:
    AutonomousHybridProbe(...)
        : DisasterTelemetryDevice(...), // Most derived class constructs virtual base
          SolarPoweredNode(...),
          WirelessLoRaTransceiver(...) {}
};
```

---

### D. Runtime Polymorphism & Dynamic Dispatch
* **Concept**: Resolving overridden methods at runtime through base pointers (`BaseSensorNode*`).
* **Mechanism**:
  - The compiler generates a **vtable** containing addresses of virtual functions.
  - Each object holds a hidden **vptr** pointing to its class's vtable.
  - Calling `node->readTelemetry()` resolves through the `vptr` at runtime.
* **Code Reference**: `SensorNode.cpp` (`runSensorNetworkSweep`)
  ```cpp
  void runSensorNetworkSweep(std::vector<BaseSensorNode*>& nodes) {
      for (BaseSensorNode* node : nodes) {
          node->readTelemetry();     // Dynamic dispatch
          node->evaluateThreshold(); // Dynamic dispatch
      }
  }
  ```

---

## 6. UNIT IV: Streams, Persistence, Exception Handling & STL

### A. Custom Exception Hierarchy
* **Concept**: Derived from `std::exception`, overriding `what() const noexcept`.
* **Code Reference**: `Exceptions.h`
  - `SensorFailureException`: Hardware communication or file write failures.
  - `TelemetryChecksumMismatch`: Data corruption detected during packet parsing.
  - `ThresholdOverflowException`: Critical physical safety thresholds exceeded.

---

### B. Binary File Handling & POD Records
* **Concept**: Direct memory serialization using fixed-byte structs (`read()` and `write()`).
* **Code Reference**: `IncidentManager.h` & `IncidentManager.cpp`
  ```cpp
  struct IncidentRecord {
      int   incidentId;
      int   stationId;
      char  regionName[64]; // Fixed char array (POD) - no std::string pointers!
      char  hazardType[32];
      float hazardIndex;
      char  riskLevel[16];
      char  timestamp[32];
  };

  // Binary write:
  ofs.write(reinterpret_cast<const char*>(&rec), sizeof(IncidentRecord));

  // Binary read:
  ifs.read(reinterpret_cast<char*>(&rec), sizeof(IncidentRecord));
  ```

---

### C. File Positioning & Transactional Rollback
* **Concept**: `seekp()` sets put/write position; `seekg()` sets get/read position.
* **Rollback Technique**:
  1. Record position before writing: `preWritePos = ofs.tellp();`
  2. Attempt binary `write()`.
  3. If `ofs.fail()` occurs, catch exception, reopen without append flag, and call `rollback.seekp(preWritePos)` to truncate uncommitted bytes.

---

### D. Stream State Resetting (`fs.clear()`)
* **Crucial Rule**: When reading a file until EOF, the stream sets `eofbit` and `failbit`.
* **Solution**: `fs.clear()` must be called before `fs.seekg(0, std::ios::beg)` to allow subsequent seek operations to succeed.

---

### E. STL Containers, Algorithms & Custom Functor
* **`std::vector`**: Dynamic arrays for sensor stations and polymorphic nodes.
* **`std::map<std::string, float>`**: Key-value associative mapping from region name to hazard index.
* **`std::sort`**: Sorts regional clusters using custom functor:
  ```cpp
  struct EvacuationPriority {
      bool operator()(const SensorCluster& a, const SensorCluster& b) const {
          return a.getAverageHazard() > b.getAverageHazard(); // Descending order
      }
  };
  std::sort(clusters.begin(), clusters.end(), EvacuationPriority{});
  ```

---

### F. Custom Stream Manipulator
* **Concept**: Stream formatting functions inserted with `<<`.
* **Code Reference**: `AlertSystem.cpp`
  ```cpp
  std::ostream& alertHeader(std::ostream& os) {
      os << "========================================\n";
      return os;
  }
  ```

---

## 7. Unit-Wise Syllabus Concept Mapping Table

| Syllabus Unit | C++ Concept | Class / Function / Construct Used | Purpose in Project |
| :--- | :--- | :--- | :--- |
| **Unit I** | Namespaces | `DisasterCore::Sensors`, `DisasterCore::Alerts` | Eliminates symbol collisions between sensors and sirens |
| **Unit I** | Inline Functions | `classifyRisk()`, `normalizeValue()` | High-performance risk categorization without call stack overhead |
| **Unit I** | Function Overloading | `computeHazardIndex(...)` (3 overloads) | Multi-hazard calculation for Landslide, Seismic, and Flood models |
| **Unit I** | Call by Reference | `SensorStation::updateReadings(float&, ...)` | In-place parameter updates without copying overhead |
| **Unit I** | Return by Reference | `SensorStation::getHazardIndexRef()` | Enables direct lvalue mutation of private hazard indices |
| **Unit I** | Friend Function | `auditPower(const PowerSubsystem&)` | Inspects private battery levels without public getters |
| **Unit I** | Friend Class | `TelemetryAuditor` inside `SensorStation` | Diagnostic audit class accessing private telemetry metrics |
| **Unit II** | Encapsulation & Const | `SensorStation`, `EarlyWarningSiren` | Data hiding with const-correct accessors |
| **Unit II** | Dynamic Memory | `new float[4]` and `delete[] readingsBuffer` | Heap allocation and deallocation for telemetry buffers |
| **Unit II** | Deep Copy Constructor | `SensorStation::SensorStation(const SensorStation&)` | Independent heap buffer duplication preventing double-frees |
| **Unit II** | Static Members | `SensorStation::totalStations`, `EarlyWarningSiren::globalAlertLevel` | Global tracking of total active stations and network alert status |
| **Unit II** | Operator Overloading (`+`) | `SensorCluster::operator+(const SensorCluster&)` | Combines regional sensor clusters into a consolidated zone |
| **Unit II** | Operator Overloading (`>`) | `SensorStation::operator>`, `SensorCluster::operator>` | Compares stations and clusters by risk severity |
| **Unit II** | Operator Overloading (`<<`, `>>`) | `operator<<(ostream&, ...)`, `operator>>(istream&, ...)` | Formatted stream insertion and extraction for console reporting |
| **Unit III** | Abstract Base Class | `BaseSensorNode` (with pure virtuals `= 0`) | Base polymorphic contract for field probes |
| **Unit III** | Virtual Inheritance | `SolarPoweredNode`, `WirelessLoRaTransceiver : virtual DisasterTelemetryDevice` | Resolves the diamond problem in `AutonomousHybridProbe` |
| **Unit III** | Containership (Composition) | `BaseSensorNode` contains `PowerSubsystem powerUnit` | Demonstrates "has-a" relationship for solar battery power systems |
| **Unit III** | Runtime Polymorphism | `std::vector<BaseSensorNode*>`, `runSensorNetworkSweep()` | Dynamic method dispatch via vtable/vptr during network sweeps |
| **Unit IV** | Custom Exceptions | `SensorFailureException`, `TelemetryChecksumMismatch`, `ThresholdOverflowException` | Derived from `std::exception`, overriding `what()` with error context |
| **Unit IV** | Binary File Handling | `IncidentManager` (`ios::binary`, `read()`, `write()`) | Flat-file binary persistence for emergency incident logs |
| **Unit IV** | File Positioning & Rollback | `seekp()`, `seekg()`, `tellg()` in `IncidentManager` | Tracks write offsets and rolls back partially written records on failure |
| **Unit IV** | STL Containers | `std::vector`, `std::map<std::string, float>` | Stores stations, active alerts, and mapped regional risk indices |
| **Unit IV** | STL Algorithms | `std::sort`, `std::for_each`, `std::max`, `std::find_if` | Iterates stations and sorts regional clusters by risk severity |
| **Unit IV** | Custom Functor | `struct EvacuationPriority` | Comparator functor used by `std::sort` for evacuation route ranking |
| **Unit IV** | Custom Stream Manipulator | `alertHeader`, `alertFooter` | Formats alert reports with clean ASCII table separators |

---

## 8. Viva Voce & Practical Examination Defense Guide

1. **Why does `SensorStation` allocate `new float[4]` instead of using `float readings[4]`?**
   * *Answer*: To demonstrate explicit dynamic memory management on the heap, proper RAII deallocation with `delete[]`, and the **Rule of Five** (deep copy constructor and copy assignment operator to avoid shallow pointer copies and double-free crashes).

2. **How does virtual inheritance solve the Diamond Problem?**
   * *Answer*: By using `public virtual DisasterTelemetryDevice`, intermediate derived classes share a single root sub-object. The compiler ensures that the most derived class (`AutonomousHybridProbe`) constructs the root base exactly once, eliminating ambiguity when calling `getDeviceInfo()`.

3. **Why use containership instead of inheritance for `PowerSubsystem`?**
   * *Answer*: A sensor node *uses* a solar battery unit; it *is not* a battery. Containership correctly models this "has-a" relationship while keeping class responsibilities separated.

4. **Why are `char` arrays used in `IncidentRecord` instead of `std::string`?**
   * *Answer*: `std::string` allocates its character buffer on the heap and contains internal pointers. Writing a `std::string` directly to disk with binary `write()` would write meaningless memory addresses. Fixed-size `char` arrays produce a **Plain Old Data (POD)** struct with a contiguous binary memory layout that safely serializes to disk.

5. **Why is `fs.clear()` necessary before seeking in `loadSensorStations`?**
   * *Answer*: When a file stream reads to EOF, it sets its `eofbit` and `failbit`. Once set, stream operations like `seekg()` silently fail unless `fs.clear()` is called first to reset the error flags.

6. **What is dynamic dispatch and how does it operate in `runSensorNetworkSweep`?**
   * *Answer*: Dynamic dispatch resolves overridden virtual functions at runtime. Each class with virtual methods has a **vtable** storing function pointers. Objects contain a hidden **vptr** pointing to their class's vtable. When invoking `node->readTelemetry()`, the program indexes through the `vptr` to call the derived class's implementation.
