#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

using namespace std;
enum
{
	Timeout = 1,                  // [msec]
	EachTimeout = 2,              // [msec]
	bufferSize = 8192,//8K
};

HANDLE HCom = INVALID_HANDLE_VALUE;
int ReadableSize = 0;
char ErrorMessage[32] = "no error.";
unsigned char send_buffer[bufferSize];
unsigned char receive_buffer[bufferSize];

int changeBaudrate(long baudrate)
{
	DCB dcb;
	GetCommState(HCom, &dcb);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.fParity = FALSE;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(HCom, &dcb);
	return 0;
}

int connect_serial(const char* device, long baudrate)
{// currently only support baud rate 115200
	char adjust_device[16];
	_snprintf(adjust_device, 16, "\\\\.\\%s", device);
	HCom = CreateFileA(adjust_device, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (HCom == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	return changeBaudrate(baudrate);
}


void disconnect_serial(void)
{
	if (HCom != INVALID_HANDLE_VALUE)
	{
		CloseHandle(HCom);
		HCom = INVALID_HANDLE_VALUE;
	}
}

int read_serial(unsigned char* data, int max_size, int timeout)
{
	if (max_size <= 0)
	{
		return 0;
	}

	if (ReadableSize < max_size)
	{
		DWORD dwErrors;
		COMSTAT ComStat;
		ClearCommError(HCom, &dwErrors, &ComStat);
		ReadableSize = ComStat.cbInQue;
	}

	if (max_size > ReadableSize)
	{
		COMMTIMEOUTS pcto;
		int each_timeout = 2;

		if (timeout == 0)
		{
			max_size = ReadableSize;

		}
		else
		{
			if (timeout < 0)
			{
				/* If Timeout is 0, this function wait data infinity */
				timeout = 0;
				each_timeout = 0;
			}

			/* set Timeout */
			GetCommTimeouts(HCom, &pcto);
			pcto.ReadIntervalTimeout = timeout;
			pcto.ReadTotalTimeoutMultiplier = each_timeout;
			pcto.ReadTotalTimeoutConstant = timeout;
			SetCommTimeouts(HCom, &pcto);
		}
	}

	DWORD n;
	ReadFile(HCom, data, (DWORD)max_size, &n, NULL);

	if (n > 0)
	{
		ReadableSize -= n;
	}

	return n;
}
