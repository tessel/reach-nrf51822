Command = {
    // Base Addr
    BASE:                 0x80,

    // General Ops
    NOP:                  0x00,
    SLEEP:                0x10,

    // SPI
    SPICMDBASE:           0x20,
    SPIENABLE:            0x20,
    SPITRANSFER:          0x21,
    SPIDISABLE:           0x22,
    SPISETCLOCKDIVISOR:   0x23,
    SPISETMODE:           0x24,
    SPISETROLE:           0x25,
    SPISETFRAME:          0x26,

    // I2C
    I2CCMDBASE:           0x30,
    I2CENABLE:            0x30,
    I2CWRITE:             0x31,
    I2CREAD:              0x32,
    I2CDISABLE:           0x33,
    I2CSETMODE:           0x34,
    I2CSETSLAVEADDRESS:   0x35,

    // UART
    UARTCMDBASE:          0x40,
    UARTENABLE:           0x40,
    UARTTRANSFER:         0x41,
    UARTRECEIVE:          0x42,
    UARTDISABLE:          0x43,
    UARTSETBAUDRATE:      0x44,
    UARTSETDATABITS:      0x45,
    UARTSETPARITY:        0x46,
    UARTSETSTOPBITS:      0x47,

    // GPIO
    GPIOCMDBASE:          0x50,
    GPIOSETPULL:          0x50,
    GPIOSETSTATE:         0x51,
    GPIOWRITEPWMVALUE:    0x52,
    GPIOGETPULL:          0x53,
    GPIOGETSTATE:         0x54,
    GPIOREADPULSELENGTH:  0x55,
    GPIOSETINTERRUPTMODE: 0x56,
    GPIOGETINTERRUPTMODE: 0x57
};

NO_CHANGE = 0xFF;

CommState = {
    SPIEnable: "SPIEnable",
    I2CEnable: "I2CEnable",
    UARTEnable: "UARTEnable",
    Idle: "Idle"
};

module.exports.Command = Command;
module.exports.NO_CHANGE = NO_CHANGE;
module.exports.CommState = CommState;
