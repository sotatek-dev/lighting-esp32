# YamahaLightingEsp32

A project for porting Block B and C code to the ESP32 with FreeRTOS integration.

## Prerequisites

- ESP-IDF v5.3
- ESP32-S3 development board 
- Visual Studio Code with ESP-IDF extension
- Python 3.11+
- Git

## Installation & Setup

### 1. Install ESP-IDF

```bash
# Windows Installation
cd C:/esp
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.bat
```

### 2. Configure VS Code

1. Install ESP-IDF extension from VS Code marketplace
2. Open VS Code settings (Ctrl+,)
3. Search for "ESP-IDF" 
4. Set ESP-IDF path to your installation directory (e.g. `C:/esp/esp-idf`)

### 3. Project Setup

```bash
# Clone project
git clone [project-url]
cd YamahaLightingEsp32

# Configure project
idf.py set-target esp32s3
idf.py menuconfig
```

In menuconfig:
1. Navigate to `Component config → FreeRTOS`
2. Enable FreeRTOS support
3. Save configuration

## Building & Flashing

### Build Project
```bash
idf.py build
```

### Flash to Device
```bash
# Replace COM3 with your device's port
idf.py -p COM3 flash 
```

### Monitor Output
```bash
idf.py -p COM3 monitor
```

Press `Ctrl+]` to exit monitor mode.

## Project Structure

```
YamahaLightingEsp32/
├── main/
│   ├── BlockB++/           # Core Block B logic
│   │   ├── include/
│   │   └── src/
│   ├── BlockC++/           # Core Block C logic
│   │   ├── include/
│   │   └── src/
│   ├── Eigen/              # Eigen library
│   ├── nlohmann/           # Nlohmann library (only need json.hpp)
│   ├── main_freeRTOS.cpp  # Main application file
│   └── CMakeLists.txt
├── sdkconfig               # Project configuration
└── CMakeLists.txt         # Build configuration
```
