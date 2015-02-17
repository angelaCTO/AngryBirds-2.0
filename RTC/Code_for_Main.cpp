/*
* Code to include in main to read time from RTC on Beaglebone Black
*/

#include <string>
#include <iostream>
#include <stdio.h>

string real_time; // the time from the RTC

int main(){
	
    real_time = exec(“hwclock -r -f /dev/rtc1”); // yeah, that’s it

}

// Execute and read terminal command 
std::string exec(char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}