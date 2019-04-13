#include "main.h"
#include "cmsis_os.h"
#include "dwt_delay.h"

// Change this to your pin
#define TX_PORT GPIOB
#define TX_PIN GPIO_PIN_11

static uint8_t symbols[] = { 0xd, 0xe, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 0x23,
		0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34 };

#define lo8(x) ((x)&0xff)
#define hi8(x) ((x)>>8)

void txbyte(const char *data) {
	for (int bt = 0; bt < 2; bt++) {
		for (int bi = 0; bi < 6; bi++) {
			if (data[bt] & (1 << bi))
				HAL_GPIO_WritePin(TX_PORT, TX_PIN, GPIO_PIN_SET);
			else
				HAL_GPIO_WritePin(TX_PORT, TX_PIN, GPIO_PIN_RESET);
			DWT_Delay(500);
		}
	}
}

uint16_t RHcrc_ccitt_update(uint16_t crc, uint8_t data) {
	data ^= lo8(crc);
	data ^= data << 4;

	return ((((uint16_t) data << 8) | hi8(crc)) ^ (uint8_t) (data >> 4)
			^ ((uint16_t) data << 3));
}

int rftx(char *data, int length) {
	char buffer[2];
	const char preamble[] = { 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x38, 0x2c };
	const uint8_t truelen = length + 3 + 4;
	uint16_t crc = 0xFFFF;

	uint8_t byte;

	if (length > 60) {
		return 0;
	}

	for (int i = 0; i < 4; i++) {
		txbyte(&preamble[i * 2]);
	}

	crc = RHcrc_ccitt_update(crc, truelen);
	buffer[0] = symbols[truelen >> 4];
	buffer[1] = symbols[truelen & 0xf];
	txbyte(buffer);

	for (int i = 0; i < 2; i++) {
		byte = 255;
		crc = RHcrc_ccitt_update(crc, byte);
		buffer[0] = symbols[byte >> 4];
		buffer[1] = symbols[byte & 0xf];
		txbyte(buffer);
	}
	for (int i = 0; i < 2; i++) {
		byte = 0;
		crc = RHcrc_ccitt_update(crc, byte);
		buffer[0] = symbols[byte >> 4];
		buffer[1] = symbols[byte & 0xf];
		txbyte(buffer);
	}

	for (int i = 0; i < length; i++) {
		crc = RHcrc_ccitt_update(crc, data[i]);
		buffer[0] = symbols[data[i] >> 4];
		buffer[1] = symbols[data[i] & 0xf];
		txbyte(buffer);
	}

	crc = ~crc;
	buffer[0] = symbols[(crc >> 4) & 0xf];
	buffer[1] = symbols[crc & 0xf];
	txbyte(buffer);
	buffer[0] = symbols[(crc >> 12) & 0xf];
	buffer[1] = symbols[(crc >> 8) & 0xf];
	txbyte(buffer);

	HAL_GPIO_WritePin(TX_PORT, TX_PIN, GPIO_PIN_RESET);
	return 1;
}
