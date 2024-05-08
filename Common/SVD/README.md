This folder contains svd files, which describe the registers of the MCU.  They are used by Ozone to show detailed information for the MCU's peripherals.

Each filename is the name of an MCU, followed by `.svd`.  Each supported MCU has one `*.jdebug` file, which instructs Ozone how to flash the MCU and load the correct SVD file so the MCU peripheral register map is accurate.