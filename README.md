# Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG)

A menu-driven C++ application for disaster monitoring and early warning in mountainous regions (e.g., the Himalayas). The system monitors simulated IoT telemetry, calculates composite hazard risks, triggers regional sirens, prioritizes evacuation corridors, and maintains persistent incident records using binary file handling.

---

## 1. Project Introduction

Mountainous regions, particularly the young fold mountains of the Himalayas, are prone to multi-hazard disasters such as rainfall-triggered landslides, flash floods, and seismic slope instabilities. Traditional monitoring systems often operate in siloes, failing to combine cross-sensor metrics (soil moisture, seismic vibration, slope inclination, and river stage heights).

The **Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG)** provides an integrated, modular simulation platform. It models autonomous sensor stations deployed across mountain sectors, computes specialized and composite hazard indices, activates early warning sirens, sorts regions by evacuation urgency, and logs high-priority incidents to non-volatile binary storage.

---

## 2. Problem Statement

Early warning mechanisms in rugged mountain terrain face three critical engineering challenges:
1. **Heterogeneous Telemetry**: Sensor readings come from diverse sources (acoustic tilt, seismic vibrations, river gauges, soil saturation probes) requiring flexible, polymorphic abstractions.
2. **Real-Time Risk Fusion**: Individual sensor spikes may not indicate imminent disaster; composite indices that cross-correlate factors (e.g., rainfall rate + slope angle vs. soil saturation + vibration) are required.
3. **Data Integrity & Persistence**: Emergency records must survive application restarts and system crashes without relying on heavy external database engines.

---

## 3. Project Objectives

- Implement an Object-Oriented software architecture adhering strictly to the four core units of C++ OOP.
- Provide separate, mathematical risk calculation overloads for landslide, seismic, and flash-flood scenarios.
- Classify threats into four actionable tiers: `LOW`, `MODERATE`, `HIGH`, and `CRITICAL`.
- Simulate regional early warning sirens and maintain an acknowledged/active alert queue.
- Use operator overloading (`+` and `>`) to merge regional sensor clusters and prioritize evacuation zones.
- Implement persistent binary file handling with transactional rollback mechanisms.
- Demonstrate modern C++ dynamic polymorphism and solve diamond inheritance ambiguity cleanly.

---

## 4. Key Features

- **Module A: Sensor Station Management**: Dynamic station registration, individual telemetry inspection, friend-class auditing, deep-copy cloning, call-by-reference updates, and file export/import.
- **Module B: Hazard Detection & Risk Calculation**: Overloaded calculation functions for Landslide Risk, Seismic/Soil Saturation Risk, and Flash-Flood Risk, producing a normalized 0–100 Composite Hazard Index.
- **Module C: Early Warning & Siren Alert System**: Global alert level tracker (`Normal`, `Watch`, `Warning`, `Emergency`), siren simulation, alert queue with acknowledgment, and custom stream manipulators.
- **Module D: Regional Risk Management**: Regional clustering with `operator+` for cluster concatenation, `operator>` for hazard comparison, and descending risk sorting.
- **Module E: Evacuation Priority List**: Functor-based ranking (`EvacuationPriority`) outputting an evacuation roadmap prioritizing the most critical sectors.
- **Module F: Incident History & Reports**: Binary flat-file storage (`incident_history.dat`) using `read()`/`write()` with `seekg()`/`seekp()`, file rollback on failure, and custom exception handling.
- **Module G: Sensor Network Sweep**: Polymorphic array execution across diverse sensor probes via pure virtual interfaces (`BaseSensorNode*`).
- **Module H: System Statistics**: Real-time summary showing active stations, sirens, global severity levels, and region hazard distributions.

---

## 5. Technologies Used

- **Language**: Standard C++ (C++17)
- **Compiler**: Clang / GCC (`g++ -std=c++17`)
- **Libraries**: C++ Standard Library (no external frameworks, AI libraries, or cloud dependencies)
  - Containers: `std::vector`, `std::map`, `std::pair`
  - Algorithms: `std::sort`, `std::for_each`, `std::max`, `std::min`
  - Streams: `<iostream>`, `<iomanip>`, `<fstream>`
  - Diagnostics: `<exception>`, `<cstring>`, `<ctime>`

---

## 6. Class Hierarchy & Architecture

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

### Explanation of Core Classes

1. **`DisasterCore::Sensors::SensorStation`**:
   Represents a deployed station with dynamic telemetry buffer allocation (`new float[4]`), deep copy constructor, destructor (`delete[]`), static station counter, call-by-reference updates, return-by-reference hazard modifiers, and stream operators (`<<`, `>>`).
2. **`DisasterCore::PowerSubsystem`**:
   Composition member representing battery and solar input. Demonstrates call by reference (`setValues`) and friend function (`auditPower`).
3. **`DisasterCore::Sensors::BaseSensorNode`**:
   Abstract base class with pure virtual functions `readTelemetry()` and `evaluateThreshold()`. Embeds a `PowerSubsystem` object to demonstrate containership (has-a) vs. inheritance (is-a).
4. **`DisasterCore::Sensors::DisasterTelemetryDevice` & Diamond Children**:
   Virtual base class inherited by `SolarPoweredNode` and `WirelessLoRaTransceiver`, unified cleanly in `AutonomousHybridProbe` without duplicate sub-objects or ambiguity.
5. **`DisasterCore::Alerts::EarlyWarningSiren`**:
   Manages regional siren activation, global emergency level, and active alert queue.
6. **`DisasterCore::Alerts::SensorCluster`**:
   Demonstrates operator overloading (`+` to merge clusters, `>` to compare hazard indices).
7. **`DisasterCore::IncidentManager`**:
   Manages binary file I/O (`incident_history.dat`) with seek pointer repositioning and rollback.
8. **`DisasterCore::SensorDataManager`**:
   Manages text file I/O (`sensor_data.dat`) for station metrics.

---

## 7. Syllabus Unit-Wise Implementation Details

### UNIT I – Core Foundations and Call Semantics
- **Namespaces**: `DisasterCore::Sensors` encapsulates all stations and probe hierarchies; `DisasterCore::Alerts` encapsulates siren, cluster, and evacuation logic.
- **Inline Functions**: `classifyRisk(float index)` and `normalizeValue(float val, float maxVal)` defined inline in `SensorStation.h`.
- **Function Overloading**:
  - `computeHazardIndex(float rainfall, float slope)`: Calculates landslide hazard.
  - `computeHazardIndex(float soil, float vib, SeismicTag)`: Calculates seismic/soil instability hazard (tag-dispatched).
  - `computeHazardIndex(float river, float rainfall, float slope)`: Calculates flash-flood hazard.
- **Call by Reference**:
  - `SensorStation::updateReadings(float&, float&, float&, float&, float&)` updates telemetry in-place.
  - `PowerSubsystem::setValues(float& battery, float& solar)` sets and returns clamped power values via references.
- **Return by Reference**: `SensorStation::getHazardIndexRef()` returns `float&` allowing direct in-place mutation of the hazard index.
- **Friend Function**: `auditPower(const PowerSubsystem& ps)` in `PowerSubsystem.h` reads private battery levels for diagnostic audits.
- **Friend Class**: `TelemetryAuditor` declared as a friend inside `SensorStation`, directly reading private sensor attributes.

### UNIT II – Encapsulation, Dynamic Memory, and Operator Overloading
- **Encapsulation & Const**: Data members are strictly private; getters and inspectors are `const`.
- **Static Members**: `SensorStation::totalStations` tracks active nodes; `EarlyWarningSiren::globalAlertLevel` maintains network alert status.
- **Dynamic Memory Allocation**: `SensorStation` allocates `readingsBuffer = new float[4]` in the constructor and releases it via `delete[]` in the destructor.
- **Deep Copy Constructor**: `SensorStation(const SensorStation& other)` allocates a fresh `new float[4]` buffer and copies elements individually.
- **Operator Overloading**:
  - Binary `+`: Merges two `SensorCluster` objects, summing total hazards and concatenating station IDs.
  - Relational `>`: Compares two `SensorStation` or `SensorCluster` objects based on hazard severity.
  - Stream `<<` & `>>`: Formatted insertion and extraction operators for `SensorStation` and `SensorCluster`.

### UNIT III – Inheritance, Containership, and Polymorphism
- **Abstract Base Class**: `BaseSensorNode` defines pure virtual methods `readTelemetry()` and `evaluateThreshold()`.
- **Containership (Composition)**: `BaseSensorNode` contains a `PowerSubsystem powerUnit` object, modeling a "has-a" relationship distinct from inheritance.
- **Diamond Inheritance**: `AutonomousHybridProbe` inherits from `SolarPoweredNode` and `WirelessLoRaTransceiver`, both of which virtually inherit `DisasterTelemetryDevice`. Resolves the diamond ambiguity so `getDeviceInfo()` is called without duplicate sub-objects.
- **Runtime Polymorphism**: `std::vector<BaseSensorNode*>` stores pointers to `SoilMoistureSensor`, `SeismicVibrationSensor`, `AcousticTiltSensor`, and `RiverLevelSensor`. `runSensorNetworkSweep()` invokes overridden methods via vptr/vtable dynamic dispatch.

### UNIT IV – Streams, Persistence, Exception Handling, and STL
- **Custom Exceptions**:
  - `SensorFailureException`: Hardware/communication failure.
  - `TelemetryChecksumMismatch`: Data corruption detected during reception.
  - `ThresholdOverflowException`: Reading exceeds critical physical boundary.
- **Binary File Handling**: `IncidentManager` writes and reads POD `IncidentRecord` structs using `ofstream::write` and `ifstream::read` with `ios::binary`.
- **File Positioning & Rollback**: Uses `seekp()` and `seekg()` to determine file sizes and record offsets. Implements a rollback mechanism that truncates partial writes on error.
- **STL Containers & Algorithms**:
  - `std::vector`: Stores stations, alerts, and polymorphic nodes.
  - `std::map`: Maps region names to composite risk indices.
  - `std::sort`: Sorts regions and clusters descending by danger level.
  - `std::for_each`: Applies formatting lambdas to station lists.
- **Functor**: `EvacuationPriority` implements `bool operator()(const SensorCluster&, const SensorCluster&)` for prioritized evacuation sorting.
- **Custom Stream Manipulators**: `alertHeader` and `alertFooter` format siren and cluster reports into structured banners.

---

## 8. Unit-Wise Concept Mapping Table

| Syllabus Unit | C++ Concept | Class / Function / Construct Used | Purpose in Project |
| :--- | :--- | :--- | :--- |
| **Unit I** | Namespaces | `DisasterCore::Sensors`, `DisasterCore::Alerts` | Prevents identifier collisions between sensor modules and alert managers |
| **Unit I** | Inline Functions | `classifyRisk()`, `normalizeValue()` in `SensorStation.h` | Fast risk level translation without function call stack overhead |
| **Unit I** | Function Overloading | `computeHazardIndex(...)` (3 overloaded signatures) | Calculates distinct hazard scores for Landslide, Seismic, and Flood threats |
| **Unit I** | Call by Reference | `SensorStation::updateReadings(float&, ...)`, `PowerSubsystem::setValues(float&, ...)` | In-place telemetry synchronization avoiding object copying |
| **Unit I** | Return by Reference | `SensorStation::getHazardIndexRef()` | Directly updates composite hazard scores inside station objects |
| **Unit I** | Friend Function | `auditPower(const PowerSubsystem&)` | Diagnostic function inspecting private battery levels without public getters |
| **Unit I** | Friend Class | `TelemetryAuditor` inside `SensorStation` | Directly inspects private station telemetry during technical audits |
| **Unit II** | Constructors & Destructors | `SensorStation::SensorStation(...)`, `~SensorStation()` | Resource Acquisition Is Initialization (RAII) for sensor stations |
| **Unit II** | Dynamic Memory | `new float[4]` and `delete[] readingsBuffer` | Dynamic allocation of telemetry readings buffer |
| **Unit II** | Deep Copy Constructor | `SensorStation::SensorStation(const SensorStation&)` | Ensures independent memory buffers when cloning stations |
| **Unit II** | Static Members | `SensorStation::totalStations`, `EarlyWarningSiren::globalAlertLevel` | Global tracking of active station count and network alert state |
| **Unit II** | Operator Overloading (`+`) | `SensorCluster::operator+(const SensorCluster&)` | Combines two regional sensor clusters into a unified sector |
| **Unit II** | Operator Overloading (`>`) | `SensorStation::operator>`, `SensorCluster::operator>` | Compares stations and clusters by risk severity |
| **Unit II** | Operator Overloading (`<<`, `>>`) | `operator<<(ostream&, ...)`, `operator>>(istream&, ...)` | Formatted stream insertion and extraction for console and file I/O |
| **Unit III** | Abstract Base Class | `BaseSensorNode` (with `= 0` pure virtuals) | Polymorphic interface for all physical field probes |
| **Unit III** | Virtual Inheritance | `SolarPoweredNode`, `WirelessLoRaTransceiver : virtual DisasterTelemetryDevice` | Resolves diamond problem for `AutonomousHybridProbe` |
| **Unit III** | Containership (Composition) | `BaseSensorNode` contains `PowerSubsystem powerUnit` | Models "has-a" relationship for solar battery power systems |
| **Unit III** | Runtime Polymorphism | `std::vector<BaseSensorNode*>`, `runSensorNetworkSweep()` | Dynamic method dispatch via vtable/vptr during network sweeps |
| **Unit IV** | Custom Exceptions | `SensorFailureException`, `TelemetryChecksumMismatch`, `ThresholdOverflowException` | Structured error handling for sensor and file failures |
| **Unit IV** | Binary File Handling | `IncidentManager` (`ios::binary`, `read()`, `write()`) | Permanent flat-file persistence for emergency incidents |
| **Unit IV** | File Positioning & Rollback | `seekp()`, `seekg()`, `tellg()` in `IncidentManager` | Verifies write locations and rolls back failed transactions |
| **Unit IV** | STL Containers | `std::vector`, `std::map<std::string, float>` | Station storage, cluster tracking, and regional risk maps |
| **Unit IV** | STL Algorithms | `std::sort`, `std::for_each`, `std::max`, `std::find_if` | Sorting evacuation lists and iterating station telemetry |
| **Unit IV** | Custom Functors | `struct EvacuationPriority` | Custom sorting comparator for prioritizing evacuation zones |
| **Unit IV** | Custom Stream Manipulator | `alertHeader`, `alertFooter` | Formats alert reports with clean ASCII border styling |

---

## 9. Compilation and Execution Instructions

### Prerequisites
- Operating System: macOS / Linux / Windows (with MinGW or WSL)
- Compiler: `g++` or `clang++` supporting C++17

### Build Command
Navigate to the project root directory:
```bash
cd "/Users/raunzz/oops project/ALF-EWG"
```

Compile with all warnings enabled:
```bash
g++ -std=c++17 -Wall -Wextra -O2 -o alfewg \
    main.cpp \
    SensorStation.cpp \
    SensorNode.cpp \
    AlertSystem.cpp \
    IncidentManager.cpp \
    PowerSubsystem.cpp
```

### Run Command
```bash
./alfewg
```

---

## 10. Sample Test Run and Outputs

### Main Menu
```
  ========================================
   AUTONOMOUS LANDSLIDE & FLASH-FLOOD
         EARLY WARNING GRID
            ALF-EWG v1.0
  ========================================

  1. Sensor Station Management
  2. Enter / Update Sensor Readings
  3. Perform Hazard Analysis
  4. View Active Emergency Alerts
  5. Regional Risk Analysis
  6. Evacuation Priority List
  7. View Incident History
  8. Run Complete Sensor Network Sweep
  9. System Statistics
  0. Exit
```

### Hazard Analysis & Siren Activation (Option 3 -> 1)
```
ID    Region                LS Risk   Seismic     Flood       Composite Level     
----------------------------------------------------------------------------------
101   Himalayan Sector A    13.3      22.0        11.3        22.0      LOW       
102   Himalayan Sector B    31.7      43.0        27.3        43.0      MODERATE  
103   Himalayan Sector C    60.6      69.0        53.6        69.0      HIGH      
========================================
  *** SIREN ACTIVATED ***
  Alert ID   : 1
  Station    : 103
  Region     : Himalayan Sector C
  Hazard     : LANDSLIDE
  Index      : 69.00
  Risk Level : HIGH
  Time       : Fri Nov 20 16:47:17 2026
  Global Lvl : 2 (WARNING)
========================================
104   River Valley Zone     83.9      90.0        84.2        90.0      CRITICAL  
========================================
  *** SIREN ACTIVATED ***
  Alert ID   : 2
  Station    : 104
  Region     : River Valley Zone
  Hazard     : FLASH_FLOOD
  Index      : 90.00
  Risk Level : CRITICAL
  Time       : Fri Nov 20 16:47:17 2026
  Global Lvl : 3 (EMERGENCY)
========================================
```

### Evacuation Priority List (Option 6)
```
  --- EVACUATION PRIORITY LIST ---

========================================
   EVACUATION PRIORITY ROADMAP          
========================================
  [Priority #1] Cluster 'River Valley Zone'
    Average Hazard Index : 90.00 [CRITICAL]
    Stations Involved    : 1
    Station IDs          : [104]
    Recommended Action   : IMMEDIATE EVACUATION via High-Ground Route

  [Priority #2] Cluster 'Himalayan Sector C'
    Average Hazard Index : 69.00 [HIGH]
    Stations Involved    : 1
    Station IDs          : [103]
    Recommended Action   : Stage 2 Evacuation to Community Shelters
========================================
```

### Polymorphic Sensor Network Sweep (Option 8)
```
========================================
   ALF-EWG Sensor Network Sweep Start  
========================================
[SoilMoistureSensor] [Node #201 | Type: SoilMoistureSensor | Location: Himalayan Sector A | Battery: 100%] | Soil Moisture: 85.00 %
[ALERT] Node #201 (Himalayan Sector A): Soil moisture 85.00% EXCEEDS safe limit of 80%! Landslide risk HIGH.
----------------------------------------
[SeismicVibrationSensor] [Node #202 | Type: SeismicVibrationSensor | Location: Himalayan Sector B | Battery: 100%] | Vibration: 18.00 Hz
[ALERT] Node #202 (Himalayan Sector B): Vibration 18.00 Hz EXCEEDS safe limit of 15 Hz! Seismic activity detected.
----------------------------------------
[AcousticTiltSensor] [Node #203 | Type: AcousticTiltSensor | Location: Himalayan Sector C | Battery: 100%] | Tilt Angle: 30.00 degrees
[OK] Node #203 (Himalayan Sector C): Tilt angle 30.00 deg — structure stable.
----------------------------------------
[RiverLevelSensor] [Node #204 | Type: RiverLevelSensor | Location: River Valley Zone | Battery: 100%] | Water Level: 10.50 m
[ALERT] Node #204 (River Valley Zone): River level 10.50 m EXCEEDS flood threshold of 8 m! Evacuate downstream zones.
----------------------------------------
   ALF-EWG Sensor Network Sweep Done   
========================================
```

### Persistent Incident History (Option 7 -> 1)
```
======================================================================
  ALF-EWG  |  Incident History Log
======================================================================
 ID       : 1
 Station  : 103
 Region   : Himalayan Sector C
 Hazard   : LANDSLIDE
 Index    : 69.00
 Risk     : HIGH
 Time     : 2026-11-20 16:47:17
----------------------------------------------------------------------
 ID       : 2
 Station  : 104
 Region   : River Valley Zone
 Hazard   : FLASH_FLOOD
 Index    : 90.00
 Risk     : CRITICAL
 Time     : 2026-11-20 16:47:17
----------------------------------------------------------------------
  Total records: 2
======================================================================
```

---

## 11. Verification Checklist

- [x] Standard C++17 compilation with `-Wall -Wextra` (0 errors, 0 warnings).
- [x] All 4 units of university OOP covered and verified.
- [x] No external dependencies, AI libraries, or multi-threading.
- [x] Diamond inheritance disambiguated via `virtual` base class.
- [x] Dynamic memory allocated with `new[]` and safely freed with `delete[]`.
- [x] Deep copying independently tested and verified.
- [x] Custom exceptions derived from `std::exception` and tested with nested try-catch.
- [x] Binary file persistence (`incident_history.dat`) verified across separate program runs.
- [x] Text file handling (`sensor_data.dat`) tested for station export/import.
- [x] Robust user input handling preventing infinite loops on invalid input or EOF.
