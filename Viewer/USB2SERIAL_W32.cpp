
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
extern int sensorId;
extern bool quit;
extern int value;
extern unsigned long value1;
extern unsigned long maxMissing;
extern unsigned long maxFrame;


sensorRecord sensor;

static void short_delay(DWORD dly);

DWORD WINAPI USB2SERIAL_W32_backup(LPVOID lpParam)
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

	dcbSerialParams.BaudRate = 9600;      // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;      // Setting Parity = None 
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
	dcbSerialParams.fRtsControl = DTR_CONTROL_ENABLE;

	dcbSerialParams.fOutxCtsFlow = true;
	Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

	if (Status == FALSE)
		return ERROR_WRONG_SETTING;

	/*------------------------------------ Setting Timeouts --------------------------------------------------*/

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 500;
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
	char buffer[256];
	buffer[0] = 0x02;

	for (int i = 0; i<dNoOFBytestoWrite - 1; i++)
		buffer[i + 1] = lpBuffer[i];
	buffer[dNoOFBytestoWrite] = 0x03;

	dNoOFBytestoWrite = sizeof(lpBuffer); // Calculating the no of bytes to write into the port
	
	buffer[0] = 0x02;
	for (int i = 0; i<dNoOFBytestoWrite - 1; i++)
		buffer[i + 1] = lpBuffer[i];
	buffer[dNoOFBytestoWrite] = 0x03;



	while (quit == false) {
		
		buffer[dNoOFBytestoWrite+1] = 'x';
		Status = WriteFile(hComm,               // Handle to the Serialport
			buffer,            // Data to be written to the port 
			dNoOFBytestoWrite + 1,   // No of bytes to write into the port
			&dNoOfBytesWritten,  // No of bytes written to the port
			NULL);
		
		
		if (Status == TRUE) {
			
			printf("\n\n    %s - Written to %s size is %d ", buffer, ComPortName, dNoOfBytesWritten);
		}
		else
			return WRITE_ERROR;

		Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

		if (Status == FALSE)
			printf("\n\n    Error! in Setting CommMask");
		else
			printf("\n\n    Setting CommMask successfull");

		//exit(1);
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

			if (k > 4)
				str[4] = 0;
			value = atoi(str);
			Sleep(500);
			
		}

	}
	CloseHandle(hComm);//Closing the Serial Port
	
	//_getch();

	return 0;
}

DWORD WINAPI USB2SERIAL_W32(LPVOID lpParam) {

	CString str = ComPortName.Mid(strlen("COM"), strlen("COM") + 1);
	int comNo = _ttoi(str);

	SensorActivate(comNo, 115200, 256, 0);
	while (quit == false) {
		SensorGetValue(comNo);
		Sleep(1000);
	}

	Release();
	return 0;
}


unsigned __stdcall data_pump(void *ptr) //war static void 
{
	sensorRecord *tmp = (sensorRecord *)ptr;   /* Zeiger auf GSV-Datensatz */
	BOOL res;
	DWORD err;

	/* Solange keine Beendigung gew�nscht */
	while (!tmp->terminate)
	{
		tmp->in_count = 0;

		
		if (!tmp->in_missing)
			tmp->in_missing = 1;

		/* Lesen */
			res = ReadFile(tmp->hcom, tmp->in_buffer + tmp->in_pos,
				tmp->in_missing, &tmp->in_count, NULL); //&tmp->action_read);

			value1 += tmp->in_count;
			if(maxMissing < tmp->in_missing)
				maxMissing = tmp->in_missing;
		if (res)
		{

			if (!tmp->in_count)	//wenn nix gelesen:
				Sleep(10);	//Suspend the execution of the current thread
			else
				tmp->in_count = tmp->in_count;


		}
		else
			err = GetLastError();
		//err = res ? ERROR_SUCCESS : GetLastError();
		if (res || (tmp->clear && err == ERROR_OPERATION_ABORTED))
		{
			/* Wenn Buffer-Clear gefordert */
			if (tmp->clear)
			{
				/* Lesepuffer l�schen */
				tmp->in_count = 0;
				tmp->in_pos = 0;
				tmp->in_missing = 0;

				/* L�schanforderung zur�cksetzen */
				tmp->clear = FALSE;
				if (!SetEvent(tmp->Clr_event)) //und quittieren
					tmp->error = GetLastError(); //ERR_EVENTFAILED;
				else
					tmp->error = 0;	//evtl. alten fehler loeschen
			}
			else
			{
				/*Increase number by previously existing number*/
				/* Anzahl um vorher bereits vorhandene Anzahl erh�hen */
				tmp->in_count += tmp->in_pos;
				/* Wieder von ganz vorne beginnen */
				tmp->in_pos = 0;

				/* Eingegangene Daten analysieren */
				if (tmp->in_count)	//runtime-optim.: nur call, wenn was gelesen
					move_data(tmp);
			}
		}
		else	//ReadFile returned error:
		{	//ERROR_IO_PENDING ist kein Fehler, s. http://msdn.microsoft.com/en-us/library/aa365467%28VS.85%29.aspx
			if (err != ERROR_IO_PENDING && !tmp->clear)
			{
				tmp->error = err;
				//tmp->LastError= err | OWN_ERR_MASK;
			}
#ifdef _DEBUG
			Beep(500, 1000);
#endif
#ifdef SLEEP_IN_THREAD
			Sleep(SLEEP_IN_THREAD);
#endif
		}
	}
	if (tmp->terminate)	//TODO: Testen: ok
	{
		/* Thread beenden */
		_endthreadex(0);
		tmp->is_terminated = 1;
	}
	return 0;
}


static void move_data(sensorRecord *tmp)
{
	unsigned long j;
	unsigned char *p;

	j = tmp->in_count;   /* Anzahl Bytes */
	p = tmp->in_buffer;   /* Hier starten */
	tmp->in_missing = 0;   /* Resultat voreinstellen auf 0 */

	while (j)
	{
		sensordata t;
		unsigned long inx;
		char suffix[2];


		if (*p == DATA_KEY)
		{
			/* Sind gen�gend Daten da, um den Record vollst�ndig zu verarbeiten ? */
			if (j < FRAME_SIZE)
			{
				/* Wenn nein, Abbrechen und fehlende Byte-Anzahl angeben */
				tmp->in_missing = FRAME_SIZE - j;
				break;
			}

			/* Restanzahl berechnen */
			j -= FRAME_SIZE;
			p++;

			p++;

			memcpy((void *)&t.id, p, sizeof(unsigned int));
			p += sizeof(unsigned int);
			memset((void *)&t.value1, 0, sizeof(t.value1));

			int k = sizeof(t.value1);
			*((unsigned char *)&t.value1 + 3) = 0;
			*((unsigned char *)&t.value1 + 2) = 0;
			*((unsigned char *)&t.value1 + 1) = (p[0]);
			*((unsigned char *)&t.value1 + 0) = (p[1]);

			p += 2;

			*((unsigned char *)&t.value2 + 3) = 0;
			*((unsigned char *)&t.value2 + 2) = 0;
			*((unsigned char *)&t.value2 + 1) = (p[0]);
			*((unsigned char *)&t.value2 + 0) = (p[1]);

			/* gesch�tzten Daten-Lese-Bereich betreten */
			DWORD retv = WaitForSingleObject(tmp->io_mutex, MUTEX_MAX_WAITTIME);	//INFINITE
			if (retv == WAIT_OBJECT_0) //!= WAIT_FAILED) 
			{
				/* �berspringen, wenn Buffer clear gefordert */
				if (!tmp->clear)
				{
					/* Wert und Status in Datenpuffer legen und */
					/* Zeiger etc. anpassen */
					
					if (tmp->out_count < tmp->out_size)
						tmp->out_count++;
					else				//Buffer voll: alte Werte ueberschreiben
						tmp->out_get = (tmp->out_get + 1) % tmp->out_size;

					*(tmp->out_buffer+tmp->out_put)= t;
					//tmp->out_buffer[tmp->out_put].value1 = t.value1;
					tmp->out_put = (tmp->out_put + 1) % tmp->out_size;
					maxFrame++;
				}
				/* gesch�tzten Daten-Lese-Bereich verlassen */
				ReleaseMutex(tmp->io_mutex);

				/* ganz abbrechen, wenn Buffer clear gefordert */
				if (tmp->clear)
					return;
			}
			else
				tmp->error = ERR_MUTEXFAILED;
			//else printf("mutex gescheitert. Code: %d",tmp->error);
		}
		/* sonstige Zeichen */
		else
		{
			/* ignorieren */
			p++;
			j--;
#ifdef _DEBUG
			Beep(2000, 50);
#endif
		}
	}	//end while (j)
		/* unverarbeiteten Rest an den Anfang des Puffers kopieren */
		/* und Position zum Anf�gen setzen */
	if (j)
	{
		memmove(tmp->in_buffer, p, j);
		tmp->in_pos = j;
	}
}



void Release()
{
	DWORD wtr, i, wtrCnt;
	HANDLE tmpH;

	if (sensor.ownPtr == NULL)
		return;
	if (sensor.Prd_event)
		ResetEvent(sensor.Prd_event);

	sensor.terminate = 1;
	/*while(!gsvs[ComNo].is_terminated && wait<RELEASE_WAIT_TIME)
	{
	Sleep(1);
	wait++;
	}
	tmp->is_terminated=0; noetig?*/
	if (sensor.thread)
	{
#ifdef DUPLICATE_THREAD_HANDLE
		tmpH = gsvs[ComNo].thread_h != NULL ? gsvs[ComNo].thread_h : (HANDLE)gsvs[ComNo].thread;
#else
		tmpH = sensor.thread;
#endif
		sensor.clear = TRUE;   /* Auch Buffer-Clear setzen zur Vereinfachung */

		if (sensor.hcom)   /* Einlesevorg�nge abbrechen */
			PurgeComm(sensor.hcom, PURGE_RXABORT | PURGE_RXCLEAR);
		/* MUTEXs freigeben, da Arrayeintr�ge bereits leer sind, */
		/* gibt es nichts mehr zu sch�tzen */
		if (sensor.io_mutex != NULL)
		{
			ReleaseMutex(sensor.io_mutex);
			sensor.io_mutex = NULL;
		}
		if (sensor.Prd_mutex != NULL)
		{
			ReleaseMutex(sensor.Prd_mutex);
			sensor.Prd_mutex = NULL;
		}
		if (sensor.hcom)
		{
			wtrCnt = PURGE_LOOP_MAX; //neu ver 1.15: sicherer wg. PurgeComm Problem bei einigen Treibern wie usbser.sys
			do
			{
				/* Sicherstellen, da� der Thread tats�chlich noch existiert */
				DWORD exitcode;
				if (GetExitCodeThread(tmpH, &exitcode))
					if (exitcode != STILL_ACTIVE)
						break;
				wtrCnt--;
				/* Warten auf Beendigung des "Hintergrund"-Threads */
				wtr = WaitForSingleObject(tmpH, TIMEOUT_PURGE);

				/* Falls Timeout: nochmal Einlesen abbrechen und weiterwarten */
				if (wtr == WAIT_TIMEOUT)
					PurgeComm(sensor.hcom, PURGE_RXABORT);

			} while (wtr == WAIT_TIMEOUT && wtrCnt>0);
		}
		/* Thread endg�ltig schlie�en */
		if (sensor.thread != NULL)
		{
			CloseHandle(sensor.thread);
			sensor.thread = NULL;
		}

	}
	if (sensor.hcom)
	{
		/* alle Operationen abbrechen und alle Puffer l�schen */
		PurgeComm(sensor.hcom, PURGE_RXABORT | PURGE_TXABORT |
			PURGE_RXCLEAR | PURGE_TXCLEAR);

		/* serielle Schnittstelle schlie�en */
		CloseHandle(sensor.hcom);
		sensor.hcom = NULL;
	}
	/* nicht mehr ben�tigte Events schlie�en */
	if (sensor.Prd_event)
	{
		CloseHandle(sensor.Prd_event);
		sensor.Prd_event = NULL;
	}
	if (sensor.Clr_event)
	{
		CloseHandle(sensor.Clr_event);
		sensor.Clr_event = NULL;
	}

	if (sensor.in_buffer)
	{
		free(sensor.in_buffer);
		sensor.in_buffer = NULL;
	}

	if (sensor.out_buffer)
	{
		free(sensor.out_buffer);
		sensor.out_buffer = NULL;
	}
	


	if (sensor.io_mutex)
	{
		ReleaseMutex(sensor.io_mutex);	//noetiG??
		sensor.io_mutex = NULL;
		//CloseHandle(gsvs[ComNo].io_mutex);
	}
	sensor.ownPtr = NULL;
	
}

static	long baud_table[] =
{
	//4800,
	9600,  19200, 38400, 57600, 115200, 230400
};

static unsigned long LastErrorGlobal;	//Errorcode, nur (noch) fuer GSV4actExt


int CALLTYPE SensorActivate(int ComNo, long Bitrate, long BufSize, long flags)
{
	int i, ret, mode;
	long max_s, bufsizetmp;
	BOOL baudok = false;
	char portstr[16];
	COMMTIMEOUTS Timeout;
	DCB dcb;
	//HANDLE tmp=NULL;
	sensorRecord *tmpGSV = NULL;
	HANDLE i_mutex = NULL;
	HANDLE rd_mutex = NULL;
	HANDLE myevent, clrevent;
	unsigned threadID;
	unsigned long wtime;

	
							/* Grunds�tzliche Zul�ssigkeit */
	/* Datenpuffer: Gr��e �berpr�fen und anlegen */
	max_s = (long)(((DWORD)-1) / FRAME_SIZE);

	if (BufSize<1)
		BufSize = 1;
	else if (BufSize > max_s)
		bufsizetmp = max_s;
	else bufsizetmp = BufSize;

	for (i = 0; i<BAUD_NO; i++)	//baudrate checken
	{
		if (baud_table[i] == Bitrate)
		{
			baudok = true;
			break;
		}
	}
	if (!baudok)
		//	RET_WRONG_PARAMETER;
	{
		LastErrorGlobal = ERR_WRONG_PARAMETER;
		return SENSOR_ERROR;
	}

	if (sensor.hcom != NULL || sensor.ownPtr != NULL)
	{
		LastErrorGlobal = ERR_COM_ALREADY_OPEN;
		return SENSOR_ERROR; 	//Schnittstelle bereits geoeffnet
	}

	i_mutex = CreateMutex(NULL, TRUE, NULL);
	if (i_mutex == NULL)
	{
		LastErrorGlobal = ERR_MUTEXFAILED;
		return SENSOR_ERROR; 	//windows gewaehrt keinen mutex
	}
	rd_mutex = CreateMutex(NULL, TRUE, NULL);
	if (rd_mutex == NULL)
	{
		LastErrorGlobal = ERR_MUTEXFAILED;
		return SENSOR_ERROR; 	//windows gewaehrt keinen mutex
	}

	myevent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (myevent == NULL)
	{
		LastErrorGlobal = ERR_EVENTFAILED;
		return SENSOR_ERROR; 	//windows gewaehrt keinen event-handler
	}
	clrevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (clrevent == NULL)
	{
		LastErrorGlobal = ERR_EVENTFAILED;
		return SENSOR_ERROR; 	//windows gewaehrt keinen event-handler
	}
	tmpGSV = (sensorRecord *)calloc(sizeof(sensorRecord), 1);	//incl =0-init
	if (!tmpGSV)
	{
		LastErrorGlobal = ERR_MEM_ALLOC;
		return SENSOR_ERROR; 	//
	}

	tmpGSV->in_buffer = (unsigned char *)malloc(IN_OWNBUFSIZE);

	if (tmpGSV->in_buffer == NULL)
	{
		LastErrorGlobal = ERR_MEM_ALLOC;
		ABORT_ACTIVATE;
	}
	//tmpGSV->in_missing = 0;
	tmpGSV->out_size = bufsizetmp; // * sizeof(long);

	tmpGSV->out_buffer = (sensordata*)malloc(tmpGSV->out_size * sizeof(sensordata));

	if (!tmpGSV->out_buffer)
	{
		LastErrorGlobal = ERR_MEM_ALLOC;
		ABORT_ACTIVATE;
	}
		//tmpGSV->out_put[i]=0; tmpGSV->out_get[i]=0;

	tmpGSV->Param_in_buf = (unsigned char*)malloc(PARAM_INBUF_SIZE);	//buffer fuer Read-Parameter
	tmpGSV->Param_in_missing = 0;	//wichtig (eig. red. wg calloc)
		//sprintf(portstr,"\\\\.\\COM%i",ComNo); //s. http://msdn2.microsoft.com/en-us/library/aa363858.aspx
	sprintf_s(portstr, 16, "\\\\.\\COM%i", ComNo);
	tmpGSV->hcom = CreateFile(CString(portstr), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (tmpGSV->hcom == INVALID_HANDLE_VALUE)
	{

		LastErrorGlobal = GetLastError() | 0x10000000;



		ABORT_ACTIVATE;
	}

	Timeout.ReadIntervalTimeout = MAXDWORD; //100; http://msdn.microsoft.com/en-us/library/aa363190%28VS.85%29.aspx:

	Timeout.ReadTotalTimeoutConstant = 0; //1000;
	Timeout.ReadTotalTimeoutMultiplier = 0; //100;
	Timeout.WriteTotalTimeoutConstant = WRITE_TIMEOUT_CONSTANT; //100;
	Timeout.WriteTotalTimeoutMultiplier = WRITE_TIMEOUT_MULTIPLIER; //2;

	if (!SetCommTimeouts(tmpGSV->hcom, &Timeout))
	{
		LastErrorGlobal = GetLastError();

		ABORT_ACTIVATE;
	}
	if (!GetCommState(tmpGSV->hcom, &dcb))
	{
		LastErrorGlobal = GetLastError();

		ABORT_ACTIVATE;
	}
	dcb.BaudRate = Bitrate;
	dcb.DCBlength = sizeof(dcb);
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = flags & ACTEX_FLAG_HANDSHAKE ? TRUE : FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = flags & ACTEX_FLAG_HANDSHAKE ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = flags & ACTEX_FLAG_HANDSHAKE ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE; //war RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;
	dcb.ByteSize = 8; //CHARACTER_SIZE;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;


	if (!SetCommState(tmpGSV->hcom, &dcb))
	{
		LastErrorGlobal = GetLastError();

		ABORT_ACTIVATE;
	}
	if (!SetupComm(tmpGSV->hcom, IN_BUFSIZE, OUT_BUFSIZE))
	{
		LastErrorGlobal = GetLastError();

		ABORT_ACTIVATE;
	}

		
	tmpGSV->terminate = FALSE;
	tmpGSV->clear = FALSE;
	/*thread-init war _beginthread(gsv_data_pump, STACK_SIZE, &gsvs[ComNo])*/
	tmpGSV->thread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, &data_pump, (sensorRecord*)&sensor, 0, &threadID); //tmpGSV);

	if (!tmpGSV->thread) // == (unsigned long)-1)
	{
		LastErrorGlobal = GetLastError();

		ABORT_ACTIVATE;
	}

	tmpGSV->Bitrate = Bitrate;
	tmpGSV->ComNr = ComNo;
	//nu erstmal alles ok: gsv in array eintragen
	sensor = *tmpGSV;
	sensor.ownPtr = &sensor; //tmpGSV;	
	sensor.io_mutex = i_mutex;
	sensor.Prd_mutex = rd_mutex;
	sensor.Prd_event = myevent;
	sensor.Clr_event = clrevent;
	/* Dann alle gesch�tzten Bereiche verlassen */
	ReleaseMutex(i_mutex);
	ReleaseMutex(rd_mutex);
	if (flags & ACTEX_FLAG_WAIT_EXTENDED)
		wtime = WAIT_ACTIVATE * WAIT_ACTIVATE_FACT;
	else
		wtime = WAIT_ACTIVATE;
	Sleep(wtime);
	/* Test auf Kommunikationsbereitschaft */
	if (flags & ACTEX_FLAG_HANDSHAKE)
	{
		DWORD mst = 0U, wtt = GetTickCount() + TIMEOUT_GSV_INIT;

		do
		{
			if (!GetCommModemStatus(tmpGSV->hcom, &mst))
			{
				mst = 0U;
				break;
			}
			else if (mst & MS_CTS_ON)
				break;

			short_delay(TIMEOUT_POLLFLOW);

		} while (GetTickCount() < wtt);

		if ((mst & MS_CTS_ON) == 0U)
		{
			//ABORT_ACTIVATE;
			LastErrorGlobal = ERR_COM_GEN_FAILURE;
			Release();
			return SENSOR_ERROR;
		}
	}
	if (flags & ACTEX_FLAG_STOP_TX)
	{
		if (SensorStopTX(ComNo) == SENSOR_ERROR)
		{
			LastErrorGlobal = sensor.LastError; //LastError retten
			Release();
			return SENSOR_ERROR;
		}
		if (flags & ACTEX_FLAG_WAIT_EXTENDED)
			Sleep(WAIT_AFTER_STOPTX);
	}
	else
		sensor.txOn = true;	//Annehmen, staendige Datenuebertrgung sei an

	sensor.LastError = 0;
	LastErrorGlobal = 0;

	/*ERstzer Schnittstellen Schreib- und Lesezugriff*/

	if (flags & ACTEX_FLAG_WAIT_EXTENDED)
	{

		for (i = 0; i<TRIALS_GET_CMD; i++)
		{
			mode = SensorGetMode(ComNo);
			if (mode != SENSOR_ERROR)
				break;
		}
		if (mode == SENSOR_ERROR)
		{
			LastErrorGlobal = sensor.LastError;	//LastError retten

			Release();	//kein GSV4 gefunden
			return SENSOR_ERROR;

		}
	}
	else
	{
		if ((mode = SensorGetMode(ComNo)) == SENSOR_ERROR)
		{
			LastErrorGlobal = sensor.LastError;	//wird innerhalb der Fkt gesetzt

			Release();	//kein GSV4 gefunden
								//LastErrorGlobal= ERR_NO_GSV4_FOUND;	//
			return SENSOR_ERROR;

		}
	}

	/*
	if (mode == 0)
	{
		GSV4setMode(ComNo, 1, "berlin");
		Sleep(100);
	}
	*/
	ret = SensorGetTxMode(ComNo);
	if (ret == SENSOR_ERROR)
	{
		if (sensor.LastError != ERR_NO_GSV_ANSWER)	//ganz alte FW kennt kein GetTXmode: kein Error
		{
			LastErrorGlobal = sensor.LastError;	//wird innerhalb der Fkt gesetzt
			Release();	//kein GSV4 gefunden
								//LastErrorGlobal= ERR_NO_GSV4_FOUND;	//
			return SENSOR_ERROR;
		}
	}
	else
	{
		if (ret & 0x02)	//Bit1: aktueller TX-state
			sensor.txOn = true;
		else
			sensor.txOn = false;
	}

	/*
	ret = GSV4firmwareVersion(ComNo);
	if (ret == GSV_ERROR)
		gsvs[ComNo].FWversion = 0;
	else
		gsvs[ComNo].FWversion = ret;
	//#endif
	//LastErrorGlobal=0;
	*/
	
	return SENSOR_OK;
}


static void short_delay(DWORD dly)
{
	DWORD acnt, tcnt = GetTickCount();

	Sleep(dly);

	while ((acnt = GetTickCount() - tcnt) <= dly)
		Sleep(acnt);
}


int CALLTYPE SensorStopTX(int ComNo)
{
	int ret = SENSOR_OK;

	ret = StopTX(ComNo);

	if (ret == SENSOR_OK)
	{
		sensor.txOn = false;
		return SENSOR_OK;
	}
	else
		return SENSOR_ERROR;
}

int StopTX(int ComNo)
{
	char txbuf[2];
	DWORD Byteswritten;

	CHECK_COMNO;
	txbuf[0] = 0x23;
	if (!WriteFile(sensor.hcom, txbuf, 1, &Byteswritten, NULL))
	{
		sensor.LastError = GetLastError() | OWN_ERR_MASK;
		return SENSOR_ERROR;
	}
	if (Byteswritten != 1)
	{
		sensor.LastError = ERR_BYTES_WRITTEN;
		return SENSOR_ERROR;
	}
	return SENSOR_OK;
}

int CALLTYPE SensorGetMode(int ComNo) {
	return SENSOR_OK;
}

int CALLTYPE SensorGetTxMode(int ComNo) {
	return SENSOR_OK;
}


int CALLTYPE SensorRead(int ComNo, unsigned int *id, double* out1, double* out2)
{
	int result = SENSOR_TRUE, ix; //,err;

	CHECK_COMNO;
	
	sensorRecord *tmp = (sensorRecord *)sensor.ownPtr; //(gsvrecord *) redundant?
	
	

	if (WaitForSingleObject(tmp->io_mutex, MUTEX_MAX_WAITTIME) == WAIT_OBJECT_0) //if(EnterGSVreadMutex(ComNo))
	{
		//gsvrecord *tmp = (gsvrecord *)gsvs[ComNo].ownPtr; //(gsvrecord *) redundant?
		if (!tmp->out_count)	//hat dieser Kanal DAten?
			result = SENSOR_OK;			//nein: result=0
		if (result && !tmp->error)
		{
			*id = tmp->out_buffer[tmp->out_get].id;
			*out1 = CalcData(tmp->out_buffer[tmp->out_get].value1);
			*out2 = CalcData(tmp->out_buffer[tmp->out_get].value2);
			tmp->out_get = (tmp->out_get + 1) % tmp->out_size;
			tmp->out_count--;
			TRACE("%d \n", tmp->out_count);
		}
		ReleaseMutex(tmp->io_mutex);
	}
	else
	{
		tmp->LastError = ERR_MUTEXFAILED;
		result = SENSOR_ERROR;
	}
	if (tmp->error)
		result =SENSOR_ERROR;
	return result;
}


int CALLTYPE SensorGetValue(int ComNo)
{
	char txbuf[256];
	DWORD Byteswritten;
	int res = SENSOR_OK;

	CHECK_COMNO;
	memset(txbuf, 0, sizeof(txbuf));
	
	sprintf(txbuf, "%cGET VALUE %03d%c", STX, sensorId, ETX);



	//WRITE_BYTES_CHK_SUCC(16);

	
	int len = strlen(txbuf);
	if (!WriteFile(sensor.hcom, txbuf, (len), &Byteswritten, NULL)) 
	{	
		sensor.LastError = GetLastError() | OWN_ERR_MASK; 
		res = SENSOR_ERROR; 
	}	

	return res;
}


double CalcData(const unsigned long val) {

	printf("value is %ld \n", val);
	double ret =  ((double)val*0.0382 + 3.6165);

	return ret;
}

