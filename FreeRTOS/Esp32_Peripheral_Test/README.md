# Requirement
    This project requires the ESP-IDF toolchain. Please use the ESP-IDF PowerShell terminal to run build, flash, and monitor commands

# Build and Run Command
```
idf.py build
idf.py -p COMx flash
idf.py -p COMx monitor

```
For example, if the device is on COM3, use the following commands:
```
idf.py build
idf.py -p COM3 flash
idf.py -p COM3 monitor

```

# Folder Structure

```
├── Esp32s3_GPIO_Test
│   ├── .devcontainer
│   ├── .vscode
│   ├── CMakeLists.txt
│   ├── sdkconfig
│   └── main
|       ├── main.c  
│       └── CMakeLists.txt
├── Esp32s3_UART_Test
│   ├── .devcontainer
│   ├── .vscode
│   ├── CMakeLists.txt
│   ├── sdkconfig
│   └── main
|       ├── main.c  
│       └── CMakeLists.txt
├── Esp32s3_SPI_Test
│   ├── .devcontainer
│   ├── .vscode
│   ├── CMakeLists.txt
│   ├── sdkconfig
│   └── main
|       ├── main.c  
│       └── CMakeLists.txt
├── Readme
```
