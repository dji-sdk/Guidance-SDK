TARGET = uart_example

OBJECTS = main.o crc16.o crc32.o protocal_uart_sdk.o serial.o

CFLAGS = -g -Wall -I/usr/local/include -I./linux

LDFLAGS = -Wl,-rpath,./ -lpthread -lrt

CXX = g++

$(TARGET) : $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	
main.o : main.cpp 
	$(CXX) $(CFLAGS) -c main.cpp 
	
crc16.o : crc16.cpp crc16.h
	$(CXX) $(CFLAGS) -c crc16.cpp crc16.h
	
crc32.o : crc32.cpp crc32.h
	$(CXX) $(CFLAGS) -c crc32.cpp crc32.h
	
protocal_uart_sdk.o : protocal_uart_sdk.cpp protocal_uart_sdk.h
	$(CXX) $(CFLAGS) -c protocal_uart_sdk.cpp protocal_uart_sdk.h
	
serial.o : linux/serial.cpp linux/serial.h
	$(CXX) $(CFLAGS) -c linux/serial.cpp linux/serial.h
	
clean:
	rm *.o *.gch $(TARGET)
	
