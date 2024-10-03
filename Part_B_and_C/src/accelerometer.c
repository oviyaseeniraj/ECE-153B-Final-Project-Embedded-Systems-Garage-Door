#include "SPI.h"
#include "SysTimer.h"
#include "accelerometer.h"


void accWrite(uint8_t addr, uint8_t val){
	//data format: R/W, MB, 6 bit addr, 8 bit data
	//R/W = 0 when writing
	//MB = 0, we are doing single byte
	uint16_t data = ((uint16_t)addr << 8) | (val);
	
	// TODO access SPI_Transfer_Data
	SPI_Transfer_Data(data);
	//printf("%.2f\n", (double)data);
}

uint8_t accRead(uint8_t addr){
	//data format: R/W, MB, 6 bit addr, 8 bit data (none here)
	//R/W = 1 when reading
	//MB = 0, we are doing single byte
	// access SPI_Transfer_Data
	//uint16_t param = (0x80 << 8) | ((0x3F << 8) & ((uint16_t)addr << 8));
	//uint8_t data = (uint8_t)(SPI_Transfer_Data(param));
	uint8_t data = 0xFF & (SPI_Transfer_Data((addr | 0x80) << 8));
	//printf("%.2f \n", (double)data);
	return data;
}

void initAcc(void){
	// set full range mode
	
	//set to 100Hz
	accWrite(0x2C, 0x0A);
	
	//data format: self_test, spi = 0, int_invert, 0, full_res = 1, justify, range (2 bits)
	//full res
	accWrite(0x31, 0x08);
	
	
	// enable measurement
	accWrite(0x2D, 0x08);
}

void readValues(double* x, double* y, double* z){

	// find scaler from data sheet = 4 mg
	double scaler = 4.0/1000.0;
	
	// read values into x,y,z using accRead
	uint8_t data_x0 = accRead(0x32);
	//printf("%.2f", (double)data_x0);
	uint8_t data_x1 = accRead(0x33);
	uint8_t data_y0 = accRead(0x34);
	uint8_t data_y1 = accRead(0x35);
	uint8_t data_z0 = accRead(0x36);
	uint8_t data_z1 = accRead(0x37);
	
	// combine bytes for each axis to form twos complement representation
	int16_t x_raw = ((uint16_t)data_x1 << 8) | data_x0;
	int16_t y_raw = ((uint16_t)data_y1 << 8) | data_y0;
	int16_t z_raw = ((uint16_t)data_z1 << 8) | data_z0;
	
	// convert twos complement to signed integer
	if (x_raw > 32767) x_raw -= 65536;
	if (y_raw > 32767) y_raw -= 65536;
	if (z_raw > 32767) z_raw -= 65536;
	
	// apply scaler of 4 mg
	*x = x_raw * scaler;
	*y = y_raw * scaler;
	*z = z_raw * scaler;
	
	//printf("Acceleration: %.2f, %.2f, %.2f\r\n", *x, *y, *z);

}