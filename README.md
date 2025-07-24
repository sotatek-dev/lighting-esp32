
# Build and Run gpio_test

rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu gpio_test/
west flash


## Build and Run uart_test

rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu uart_test/
west flash
west espressif monitor

## Build and Run spi_test

rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu spi_test/
west flash
west espressif monitor

Below is short explanation of remaining files in the project folder.

```
├── gpio_test
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── README.rst
│   ├── sample.yaml
│   └── src
│       └── main.c
├── spi_test
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── README.rst
│   ├── sample.yaml
│   └── src
│       └── main.c
└── uart_test
    ├── boards
    ├── CMakeLists.txt
    ├── prj.conf
    ├── README.rst
    ├── sample.yaml
    └── src
        └── main.c
```
