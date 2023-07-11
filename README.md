# NapseFirmware

Firmware for the Napse open source EEG board.

## BLE

- Service UUID: 98abbe75-b810-4fe4-83a5-aea7dd9a915c

### Characteristics:

- Information: UUID: 71eee1da-2cef-4702-9d3e-4729e19232b5
	9 bytes, serial number and number of channels of the device
- Data: UUID: 9d746b90-552c-4eef-b3f0-506d1b3a48b2
	Ten 32 bits fields (packet number, status and the data of the 8 channels)
- Start-Stop: UUID: 5c804a1f-e0c0-4e30-b068-55a47b8a60c7
	Byte, write 0x01 to start logging and 0x00 to stop.
- Battery: UUID: 50f9da7d-8dd4-4354-956d-3d1b5d68322e
	Float32, battery voltage
- Configuration: UUID: aa766dda-0889-42ec-81f7-53cf26ad05ce
	Eight bytes, one byte per channel:
    - bit 0: normal mode (0) shortcircuit mode (1)
    - bits 1-4: gain of the channel 000 -> 110 (1,2,4,6,8,12,24)



