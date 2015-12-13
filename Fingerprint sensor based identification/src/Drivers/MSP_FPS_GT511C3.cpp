/*
	FPS_GT511C3.h v1.0 - Library for controlling the GT-511C3 Finger Print Scanner (FPS)
	Created by Josh Hawley, July 23rd 2013
	Licensed for non-commercial use, must include this license message
	basically, Feel free to hack away at it, but just give me credit for my work =)
	TLDR; Wil Wheaton's Law
*/

#include "MSP_FPS_GT511C3.hpp"
#include <stdio.h>

extern "C"{
#include "uart.h"
}

uint16_t ByteCount;
uint8_t rx_buf[RXBUF_SIZE];

// returns the 12 bytes of the generated command packet
// remember to call delete on the returned array
unsigned char* Command_Packet::GetPacketBytes()
{
	unsigned char* packetbytes= new unsigned char[12];

	// update command before calculating checksum (important!)
	unsigned long int cmd = Command;
	command[0] = GetLowByte(cmd);
	command[1] = GetHighByte(cmd);

	unsigned long int checksum = _CalculateChecksum();

	packetbytes[0] = COMMAND_START_CODE_1;
	packetbytes[1] = COMMAND_START_CODE_2;
	packetbytes[2] = COMMAND_DEVICE_ID_1;
	packetbytes[3] = COMMAND_DEVICE_ID_2;
	packetbytes[4] = Parameter[0];
	packetbytes[5] = Parameter[1];
	packetbytes[6] = Parameter[2];
	packetbytes[7] = Parameter[3];
	packetbytes[8] = command[0];
	packetbytes[9] = command[1];
	packetbytes[10] = GetLowByte(checksum);
	packetbytes[11] = GetHighByte(checksum);

	return packetbytes;
}

// Converts the int to bytes and puts them into the paramter array
void Command_Packet::ParameterFromInt(int i)
{
	Parameter[0] = (i & 0x000000ff);
	Parameter[1] = (i & 0x0000ff00) >> 8;
	Parameter[2] = (i & 0x00ff0000) >> 16;
	Parameter[3] = (i & 0xff000000) >> 24;
}

// Returns the high unsigned char from a unsigned long int
unsigned char Command_Packet::GetHighByte(unsigned long int w)
{
	return (unsigned char)(w>>8)&0x00FF;
}

// Returns the low unsigned char from a unsigned long int
unsigned char Command_Packet::GetLowByte(unsigned long int w)
{
	return (unsigned char)w&0x00FF;
}

unsigned long int Command_Packet::_CalculateChecksum()
{
	unsigned long int w = 0;
	w += COMMAND_START_CODE_1;
	w += COMMAND_START_CODE_2;
	w += COMMAND_DEVICE_ID_1;
	w += COMMAND_DEVICE_ID_2;
	w += Parameter[0];
	w += Parameter[1];
	w += Parameter[2];
	w += Parameter[3];
	w += command[0];
	w += command[1];

	return w;
}

Command_Packet::Command_Packet()
{
};


// creates and parses a response packet from the finger print scanner
Response_Packet::Response_Packet(unsigned char* buffer, bool UseSerialDebug)
{
	CheckParsing(buffer[0], COMMAND_START_CODE_1, COMMAND_START_CODE_1, "COMMAND_START_CODE_1", UseSerialDebug);
	CheckParsing(buffer[1], COMMAND_START_CODE_2, COMMAND_START_CODE_2, "COMMAND_START_CODE_2", UseSerialDebug);
	CheckParsing(buffer[2], COMMAND_DEVICE_ID_1, COMMAND_DEVICE_ID_1, "COMMAND_DEVICE_ID_1", UseSerialDebug);
	CheckParsing(buffer[3], COMMAND_DEVICE_ID_2, COMMAND_DEVICE_ID_2, "COMMAND_DEVICE_ID_2", UseSerialDebug);
	CheckParsing(buffer[8], 0x30, 0x31, "AckNak_LOW", UseSerialDebug);
	if (buffer[8] == 0x30) ACK = true; else ACK = false;
	CheckParsing(buffer[9], 0x00, 0x00, "AckNak_HIGH", UseSerialDebug);

	unsigned long int checksum = CalculateChecksum(buffer, 10);
	unsigned char checksum_low = GetLowByte(checksum);
	unsigned char checksum_high = GetHighByte(checksum);
	CheckParsing(buffer[10], checksum_low, checksum_low, "Checksum_LOW", UseSerialDebug);
	CheckParsing(buffer[11], checksum_high, checksum_high, "Checksum_HIGH", UseSerialDebug);

	Error = ErrorCodes::ParseFromBytes(buffer[5], buffer[4]);

	ParameterBytes[0] = buffer[4];
	ParameterBytes[1] = buffer[5];
	ParameterBytes[2] = buffer[6];
	ParameterBytes[3] = buffer[7];
	ResponseBytes[0]=buffer[8];
	ResponseBytes[1]=buffer[9];
	for (unsigned int i=0; i < 12; i++)
	{
		RawBytes[i]=buffer[i];
	}
}

// parses bytes into one of the possible errors from the finger print scanner
Response_Packet::ErrorCodes::Errors_Enum Response_Packet::ErrorCodes::ParseFromBytes(unsigned char high, unsigned char low)
{
	Errors_Enum e = INVALID;
	if (high == 0x00)
	{
	}
	if (high == 0x01)
	{
		switch(low)
		{
			case 0x00: e = NO_ERROR; break;
			case 0x01: e = NACK_TIMEOUT; break;
			case 0x02: e = NACK_INVALID_BAUDRATE; break;
			case 0x03: e = NACK_INVALID_POS; break;
			case 0x04: e = NACK_IS_NOT_USED; break;
			case 0x05: e = NACK_IS_ALREADY_USED; break;
			case 0x06: e = NACK_COMM_ERR; break;
			case 0x07: e = NACK_VERIFY_FAILED; break;
			case 0x08: e = NACK_IDENTIFY_FAILED; break;
			case 0x09: e = NACK_DB_IS_FULL; break;
			case 0x0A: e = NACK_DB_IS_EMPTY; break;
			case 0x0B: e = NACK_TURN_ERR; break;
			case 0x0C: e = NACK_BAD_FINGER; break;
			case 0x0D: e = NACK_ENROLL_FAILED; break;
			case 0x0E: e = NACK_IS_NOT_SUPPORTED; break;
			case 0x0F: e = NACK_DEV_ERR; break;
			case 0x10: e = NACK_CAPTURE_CANCELED; break;
			case 0x11: e = NACK_INVALID_PARAM; break;
			case 0x12: e = NACK_FINGER_IS_NOT_PRESSED; break;
		}
	}
	return e;
}

// Gets an int from the parameter bytes
int Response_Packet::IntFromParameter()
{
	int retval = 0;
	retval = (retval << 8) + ParameterBytes[3];
	retval = (retval << 8) + ParameterBytes[2];
	retval = (retval << 8) + ParameterBytes[1];
	retval = (retval << 8) + ParameterBytes[0];
	return retval;
}

// calculates the checksum from the bytes in the packet
unsigned long int Response_Packet::CalculateChecksum(unsigned char* buffer, int length)
{
	unsigned long int checksum = 0;
	for (unsigned int i=0; i<length; i++)
	{
		checksum +=buffer[i];
	}
	return checksum;
}

// Returns the high unsigned char from a unsigned long int
unsigned char Response_Packet::GetHighByte(unsigned long int w)
{
	return (unsigned char)(w>>8)&0x00FF;
}

// Returns the low unsigned char from a unsigned long int
unsigned char Response_Packet::GetLowByte(unsigned long int w)
{
	return (unsigned char)w&0x00FF;
}

// checks to see if the unsigned char is the proper value, and logs it to the serial channel if not
bool Response_Packet::CheckParsing(unsigned char b, unsigned char propervalue, unsigned char alternatevalue, char* varname, bool UseSerialDebug)
{
	bool retval = (b != propervalue) && (b != alternatevalue);
	return retval;
}

//void Data_Packet::StartNewPacket()
//{
//	Data_Packet::NextPacketID = 0;
//	Data_Packet::CheckSum = 0;
//}

// Creates a new object to interface with the fingerprint scanner


FPS_GT511C3::FPS_GT511C3(unsigned char rx, unsigned char tx)
{
	this->UseSerialDebug = false;
};


// destructor
FPS_GT511C3::~FPS_GT511C3()
{

}

//Initialises the device and gets ready for commands
bool FPS_GT511C3::Open()
{
	bool retval = false;
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Open;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();

	if (rp->ACK == false) retval = false;

	delete rp;
	delete packetbytes;
	return retval;
}

// According to the DataSheet, this does nothing...
// Implemented it for completeness.
void FPS_GT511C3::Close()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Close;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	delete rp;
	delete packetbytes;
};

// Turns on or off the LED backlight
// Parameter: true turns on the backlight, false turns it off
// Returns: True if successful, false if not
bool FPS_GT511C3::SetLED(bool on)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CmosLed;
	if (on)
	{
		cp->Parameter[0] = 0x01;
	}
	else
	{
		cp->Parameter[0] = 0x00;
	}
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	bool retval = true;
	if (rp->ACK == false) retval = false;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
};

// Changes the baud rate of the connection
// Parameter: 9600, 19200, 38400, 57600, 115200
// Returns: True if success, false if invalid baud
// NOTE: Untested (don't have a logic level changer and a voltage divider is too slow)
bool FPS_GT511C3::ChangeBaudRate(int baud)
{
	if ((baud == 9600) || (baud == 19200) || (baud == 38400) || (baud == 57600) || (baud == 115200))
	{

		Command_Packet* cp = new Command_Packet();
		cp->Command = Command_Packet::Commands::Open;
		cp->ParameterFromInt(baud);
		unsigned char* packetbytes = cp->GetPacketBytes();
		SendCommand(packetbytes, 12);
		Response_Packet* rp = GetResponse();
		bool retval = rp->ACK;
		if (retval)
		{
			// _serial.end();
			// _serial.begin(baud);
		}
		delete rp;
		delete packetbytes;
		return retval;
	}
	return false;
}

// Gets the number of enrolled fingerprints
// Return: The total number of enrolled fingerprints
int FPS_GT511C3::GetEnrollCount()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetEnrollCount;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();

	int retval = rp->IntFromParameter();
	delete rp;
	delete packetbytes;
	return retval;
}

// checks to see if the ID number is in use or not
// Parameter: 0-199
// Return: True if the ID number is enrolled, false if not
bool FPS_GT511C3::CheckEnrolled(int id)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CheckEnrolled;
	cp->ParameterFromInt(id);
	unsigned char* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = false;
	retval = rp->ACK;
	delete rp;
	return retval;
}

// Starts the Enrollment Process
// Parameter: 0-19
// Return:
//	0 - ACK
//	1 - Database is full
//	2 - Invalid Position
//	3 - Position(ID) is already used
int FPS_GT511C3::EnrollStart(int id)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::EnrollStart;
	cp->ParameterFromInt(id);
	unsigned char* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	int retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_DB_IS_FULL) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_ALREADY_USED) retval = 3;
	}
	delete rp;
	return retval;
}

// Gets the first scan of an enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int FPS_GT511C3::Enroll1()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll1;
	unsigned char* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Second scan of an enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int FPS_GT511C3::Enroll2()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll2;
	unsigned char* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Third scan of an enrollment
// Finishes Enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int FPS_GT511C3::Enroll3()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll3;
	unsigned char* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Checks to see if a finger is pressed on the FPS
// Return: true if finger pressed, false if not
bool FPS_GT511C3::IsPressFinger()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::IsPressFinger;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	bool retval = false;
	int pval = rp->ParameterBytes[0];
	pval += rp->ParameterBytes[1];
	pval += rp->ParameterBytes[2];
	pval += rp->ParameterBytes[3];
	if (pval == 0) retval = true;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
}

// Deletes the specified ID (enrollment) from the database
// Parameter: 0-199 (id number to be deleted)
// Returns: true if successful, false if position invalid
bool FPS_GT511C3::DeleteID(int id)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteID;
	cp->ParameterFromInt(id);
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
}

// Deletes all IDs (enrollments) from the database
// Returns: true if successful, false if db is empty
bool FPS_GT511C3::DeleteAll()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteAll;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
}

// Checks the currently pressed finger against a specific ID
// Parameter: 0-199 (id number to be checked)
// Returns:
//	0 - Verified OK (the correct finger)
//	1 - Invalid Position
//	2 - ID is not in use
//	3 - Verified FALSE (not the correct finger)
int FPS_GT511C3::Verify1_1(int id)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Verify1_1;
	cp->ParameterFromInt(id);
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	int retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_NOT_USED) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_VERIFY_FAILED) retval = 3;
	}
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
}

// Checks the currently pressed finger against all enrolled fingerprints
// Returns:
//	0-199: Verified against the specified ID (found, and here is the ID number)
//	200: Failed to find the fingerprint in the database
int FPS_GT511C3::Identify1_N()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Identify1_N;
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval > 200) retval = 200;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;
}

// Captures the currently pressed finger into onboard ram use this prior to other commands
// Parameter: true for high quality image(slower), false for low quality image (faster)
// Generally, use high quality for enrollment, and low quality for verification/identification
// Returns: True if ok, false if no finger pressed
bool FPS_GT511C3::CaptureFinger(bool highquality)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CaptureFinger;
	if (highquality)
	{
		cp->ParameterFromInt(1);
	}
	else
	{
		cp->ParameterFromInt(0);
	}
	unsigned char* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetbytes;
	delete cp;
	return retval;

}

// Sends the command to the software serial channel
void FPS_GT511C3::SendCommand(unsigned char cmd[], int length)
{
	UartSendLen(cmd, length);
};

// Gets the response to the command from the software serial channel (and waits for it)
Response_Packet* FPS_GT511C3::GetResponse()
{
	bool done = false;
	while (done == false)
	{
		ByteCount = UartReceiveBytesInBuffer(rx_buf);

		if (rx_buf[0] == Response_Packet::COMMAND_START_CODE_1)
		{
			done = true;
		}
	}

	Response_Packet* rp = new Response_Packet(rx_buf, UseSerialDebug);
	return rp;
};
