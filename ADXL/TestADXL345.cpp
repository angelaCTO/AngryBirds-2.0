/*
 Copyright (C) 2014  Cagdas Caglak cagdascaglak@gmail.com http://expcodes.blogspot.com.tr/

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "ADXL345.h"
#define THRESH  3
using namespace cacaosd_bbb_i2c;
using namespace cacaosd_adxl345;

int main(int argc, char **argv) {

	BBB_I2C i2c;
	ADXL345 adxl(i2c);
	adxl.initialize();
	int16_t last_x, x;
	int16_t last_y, y;
	int16_t last_z, z;
	adxl.getAcceleration(&last_x, &last_y, &last_z);
	
	printf("rate = %d\n", adxl.getRate());
	while (true) {
	    adxl.getAcceleration(&x, &y, &z);
	    /*
		printf("Raw Accel X: %d\n", x);
		printf("Raw Accel Y: %d\n", y);
		printf("Raw Accel Z: %d\n", z);
		printf("-------------------\n");
		*/
		if( (last_x > x+THRESH) | (last_y > y+THRESH) | (last_z > z+THRESH) | (last_x < x-THRESH) | (last_y < y-THRESH) | (last_z < z-THRESH) )
		{
		    printf("\n\n\t\tcollision detected!!\n\n");
		    printf("delta x = %d, delta y = %d, delta z = %d\n", x-last_x, y-last_y, z-last_z);
		}
        last_x = x;
        last_y = y;
        last_z = z;

		usleep(50000);
	}
	return 0;
}

