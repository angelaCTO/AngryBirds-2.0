 #Copyright (C) 2014  Cagdas Caglak http://expcodes.blogspot.com.tr/

 #This program is free software: you can redistribute it and/or modify
 #it under the terms of the GNU General Public License as published by
 #the Free Software Foundation, either version 3 of the License, or
 #(at your option) any later version.

 #This program is distributed in the hope that it will be useful,
 #but WITHOUT ANY WARRANTY; without even the implied warranty of
 #MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #GNU General Public License for more details.

 #You should have received a copy of the GNU General Public License
 #along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
ECHO=@echo
ECHO_N=@echo -n

EXEC_ADXL345=TestADXL345

CXX=g++
CFLAGS=-Wall -c

all: $(EXEC_ADXL345)
	@mkdir -p obj
	@mv *.o obj
	
$(EXEC_ADXL345): TestADXL345.o BBB_I2C.o ADXL345.o
	$(ECHO) "$(EXEC_ADXL345) building..."
	@$(CXX) TestADXL345.o BBB_I2C.o ADXL345.o -o $(EXEC_ADXL345)
			
TestADXL345.o: TestADXL345.cpp
	@$(CXX) $(CFLAGS) TestADXL345.cpp
	
BBB_I2C.o: BBB_I2C.cpp
	@$(CXX) $(CFLAGS) BBB_I2C.cpp

ADXL345.o: ADXL345.cpp
	@$(CXX) $(CFLAGS) ADXL345.cpp
	
clean:
	$(ECHO) "Clean."
	@rm -rf *.o $(EXEC_ADXL345) obj
