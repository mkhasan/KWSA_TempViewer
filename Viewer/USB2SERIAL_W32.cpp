
	//====================================================================================================//
	// Serial Port Programming using Win32 API in C                                                       //
	// (Writes data to serial port)                                                                      //
	//====================================================================================================//
	
	//====================================================================================================//
	// www.xanthium.in										                                              //
	// Copyright (C) 2014 Rahul.S                                                                         //
	//====================================================================================================//
	
	//====================================================================================================//
	// The Program runs on the PC side and uses Win32 API to communicate with the serial port or          //
	// USB2SERIAL board and writes the data to it.                                                       //    
	//----------------------------------------------------------------------------------------------------//
	// Program runs on the PC side (Windows) and transmits a single character.                            //            
	// Program uses CreateFile() function to open a connection serial port(COMxx).                        //
	// Program then sets the parameters of Serial Comm like Baudrate,Parity,Stop bits in the DCB struct.  //
	// After setting the Time outs,the Program writes a character to COMxx using WriteFile().             //
    //----------------------------------------------------------------------------------------------------// 
	// BaudRate     -> 9600                                                                               //
	// Data formt   -> 8 databits,No parity,1 Stop bit (8N1)                                              //
	// Flow Control -> None                                                                               //
	//====================================================================================================//

	
	//====================================================================================================//
	// Compiler/IDE  :	Microsoft Visual Studio Express 2013 for Windows Desktop(Version 12.0)            //
	//               :  gcc 4.8.1 (MinGW)                                                                 //
	// Library       :  Win32 API,windows.h,                                                              //
	// Commands      :  gcc -o USB2SERIAL_Write_W32 USB2SERIAL_Write_W32.c                                //
	// OS            :	Windows(Windows 7)                                                                //
	// Programmer    :	Rahul.S                                                                           //
	// Date	         :	30-November-2014                                                                  //
	//====================================================================================================//

	//====================================================================================================//
	// Sellecting the COM port Number                                                                     //
    //----------------------------------------------------------------------------------------------------//
    // Use "Device Manager" in Windows to find out the COM Port number allotted to USB2SERIAL converter-  // 
    // -in your Computer and substitute in the  "ComPortName[]" array.                                    //
	//                                                                                                    //
	// for eg:-                                                                                           //
	// If your COM port number is COM32 in device manager(will change according to system)                //
	// then                                                                                               //
	//			char   ComPortName[] = "\\\\.\\COM32";                                                    //
	//====================================================================================================//

#include "stdafx.h"

#include "USB2SERIAL_W32.h"

#include <stdio.h>
#include <string.h>

extern CString ComPortName;
extern bool quit;
extern int value;

DWORD WINAPI USB2SERIAL_W32(LPVOID lpParam)
{

	HANDLE hComm;                          // Handle to the Serial port
	BOOL   Status;


	hComm = CreateFile(ComPortName,                       // Name of the Port to be Opened
		GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
		0,                                 // No Sharing, ports cant be shared
		NULL,                              // No Security
		OPEN_EXISTING,                     // Open existing port only
		0,                                 // Non Overlapped I/O
		NULL);                             // Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE) {
		return INVALID_HANDLE;
	}


	/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/

	DCB dcbSerialParams = { 0 };                        // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	Status = GetCommState(hComm, &dcbSerialParams);     //retreives  the current settings

	if (Status == FALSE)
		printf("\n   Error! in GetCommState()");

	dcbSerialParams.BaudRate = CBR_9600;      // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;      // Setting Parity = None 

	Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

	if (Status == FALSE)
		return ERROR_WRONG_SETTING;

	/*------------------------------------ Setting Timeouts --------------------------------------------------*/

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(hComm, &timeouts) == FALSE)
		printf("\n   Error! in Setting Time Outs");
	else
		printf("\n\n   Setting Serial Port Timeouts Successfull");


	/*----------------------------- Writing a Character to Serial Port----------------------------------------*/
	char   lpBuffer[] = "GET VALUE 1234";		       // lpBuffer should be  char or byte array, otherwise write wil fail
	DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
	DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port

	DWORD dwEventMask;                     // Event mask to trigger
	char  TempChar;                        // Temperory Character
	char  SerialBuffer[256];               // Buffer Containing Rxed Data
	DWORD NoBytesRead;                     // Bytes read by ReadFile()

	char str[256];

	int i;

	dNoOFBytestoWrite = sizeof(lpBuffer); // Calculating the no of bytes to write into the port
	char buffer[100];
	buffer[0] = 0x02;
	for (int i = 0; i<dNoOFBytestoWrite - 1; i++)
		buffer[i + 1] = lpBuffer[i];
	buffer[dNoOFBytestoWrite] = 0x03;



	while (quit == false) {
		Status = WriteFile(hComm,               // Handle to the Serialport
			lpBuffer,            // Data to be written to the port 
			dNoOFBytestoWrite,   // No of bytes to write into the port
			&dNoOfBytesWritten,  // No of bytes written to the port
			NULL);

		if (Status == TRUE)
			printf("\n\n    %s - Written to %s", lpBuffer, ComPortName);
		else
			printf("\n\n   Error %d in Writing to Serial Port", GetLastError());



		Status = WriteFile(hComm,               // Handle to the Serialport
			buffer,            // Data to be written to the port 
			dNoOFBytestoWrite + 1,   // No of bytes to write into the port
			&dNoOfBytesWritten,  // No of bytes written to the port
			NULL);

		if (Status == TRUE)
			printf("\n\n    %s - Written to %s", lpBuffer, ComPortName);
		else
			return WRITE_ERROR;



		Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received

														   /*-------------------------- Program will Wait here till a Character is received ------------------------*/

		if (Status == FALSE)
		{
			return WAIT_EVENT_ERROR;
		}
		else //If  WaitCommEvent()==True Read the RXed data using ReadFile();
		{
			

			printf("\n\n    Characters Received");

			i = 0;
			do
			{
				Status = ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
				SerialBuffer[i] = TempChar;
				i++;
			} while (NoBytesRead > 0);



			/*------------Printing the RXed String to Console----------------------*/

			printf("\n\n    ");
			int j = 0;
			int k = 0;
			for (j = 0; j < i - 1; j++, k++) {	// j < i-1 to remove the dupliated last character 
				printf("%c", SerialBuffer[j]);
				str[k] = SerialBuffer[j];
			}
			str[k] = 0;

			value = atoi(str);
			
		}

	}
	CloseHandle(hComm);//Closing the Serial Port
	
	//_getch();

	return 0;
}
