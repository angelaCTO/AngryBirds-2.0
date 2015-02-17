CXX = g++
CXXFLAGS = -O2 -std=c++0x
CXXLIBS = 
BLACK_OBJS = /home/ubuntu/AngryBirds/BlackLib/BlackLib.o


all:$(BLACK_OBJS) analog_stream_test cron_test flash_led process_remote_signal_test


cron_test: cron_test.cpp
	$(CXX) -o cron cron_test.cpp
analog_stream_test: analog_stream.cpp $(BLACK_OBJS)
	$(CXX) -o analog_stream_test analog_stream_test.cpp $(BLACK_OBJS)
flash_led_test: flash_led,cpp $(BLACK_OBJS)
	$(CXX) -o flash_led_test flash_led.cpp $(BLACK_OBJS)
process_remote_signal_test: process_remote_signal_test.cpp $(BLACK_OBJS)
	$(CXX) -o process_remote_signal_test process_remote_signal_test.cpp $(BLACK_OBJS)

clean:
	rm -rf cron flash_led_test process_remote_signal_test analog_stream_test
