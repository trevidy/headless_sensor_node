Headless Sensor Node v1

Goal: Design a production-style embedded node capable of:
-long-term unattended operation
-power interruption recovery
-structured fault handling
-persistent configuration
-crash logging

Test platform:
ESP32-WROOM-32E

Project Goals:
-event-driven firmware architecture
-state machine controlled runtime
-persistent configuration with CRC validation
-flash-backed logging system
-watchdog supervision
-safe fault recovery

Design Principles:
-deterministic behavior
-minimal dynamic allocation
-modular firmware architecture
-recoverable failure modes
-persistent observability