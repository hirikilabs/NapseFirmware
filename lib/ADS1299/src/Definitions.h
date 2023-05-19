//
//  Definitions.h
//  
//
//  Created by Conor Russomanno on 6/19/13.
//
//

#ifndef _Definitions_h
#define _Definitions_h

// Channels
#define ADS1299_4CH 4
#define ADS1299_6CH 6
#define ADS1299_8CH 8


//SPI Command Definition Byte Assignments (Datasheet, pg. 35)
#define ADS1299_WAKEUP 0x02 // Wake-up from standby mode
#define ADS1299_STANDBY 0x04 // Enter Standby mode
#define ADS1299_RESET 0x06 // Reset the device
#define ADS1299_START 0x08 // Start and restart (synchronize) conversions
#define ADS1299_STOP 0x0A // Stop conversion
#define ADS1299_RDATAC 0x10 // Enable Read Data Continuous mode (default mode at power-up)
#define ADS1299_SDATAC 0x11 // Stop Read Data Continuous mode
#define ADS1299_RDATA 0x12 // Read data by command; supports multiple read back

#define ADS1299_RREG 0x20 // (also = 00100000) is the first opcode that the address must be added to for RREG communication
#define ADS1299_WREG 0x40 // 01000000 in binary (Datasheet, pg. 35)

//Register Addresses
#define ADS1299_ID 0x00
#define ADS1299_CONFIG1 0x01
#define ADS1299_CONFIG2 0x02
#define ADS1299_CONFIG3 0x03
#define ADS1299_LOFF 0x04
#define ADS1299_CH1SET 0x05
#define ADS1299_CH2SET 0x06
#define ADS1299_CH3SET 0x07
#define ADS1299_CH4SET 0x08
#define ADS1299_CH5SET 0x09
#define ADS1299_CH6SET 0x0A
#define ADS1299_CH7SET 0x0B
#define ADS1299_CH8SET 0x0C
#define ADS1299_BIAS_SENSP 0x0D
#define ADS1299_BIAS_SENSN 0x0E
#define ADS1299_LOFF_SENSP 0x0F
#define ADS1299_LOFF_SENSN 0x10
#define ADS1299_LOFF_FLIP 0x11
#define ADS1299_LOFF_STATP 0x12
#define ADS1299_LOFF_STATN 0x13
#define ADS1299_GPIO 0x14
#define ADS1299_MISC1 0x15
#define ADS1299_MISC2 0x16
#define ADS1299_CONFIG4 0x17


// CONFIG1
#define DR_FMOD_64      0
#define DR_FMOD_128     1
#define DR_FMOD_256     2
#define DR_FMOD_512     3
#define DR_FMOD_1024    4
#define DR_FMOD_2048    5
#define DR_FMOD_4096    6

#define CLK_EN          (1 << 5)
#define DAISY_EN        (0 << 6)
#define MULTI_READ      (1 << 6)

// CONFIG2
#define CAL_FREQ_FCLOCK_2_21    0
#define CAL_FREQ_FCLOCK_2_20    1
#define CAL_FREQ_DC             3

#define CAL_AMP_1               (0 << 2)
#define CAL_AMP_2               (1 << 2)

#define INT_CAL_EXT             (0 << 4)
#define INT_CAL_INT             (1 << 4)

// CONFIG3
#define BIAS_STAT_CONN          (0 << 0)
#define BIAS_STAT_DIS           (1 << 0)

#define BIAS_SENSE_DIS          (0 << 1)
#define BIAS_SENSE_EN           (1 << 1)

#define BIAS_REF_EXT            (0 << 3)
#define BIAS_REF_INT            (1 << 3)

#define BIAS_MEAS_OPEN          (0 << 4)
#define BIAS_MEAS_IN            (1 << 4)

#define REF_BUFF_OFF            (0 << 7)
#define REF_BUFF_ON             (1 << 7)

#endif


// CHXSET

#define PGA_GAIN_1              0b10001111
#define PGA_GAIN_2              0b10011111
#define PGA_GAIN_4              0b10101111
#define PGA_GAIN_6              0b10111111
#define PGA_GAIN_8              0b11001111
#define PGA_GAIN_12             0b11011111
#define PGA_GAIN_24             0b11101111

#define CH_PDOWN                (1 << 7)
