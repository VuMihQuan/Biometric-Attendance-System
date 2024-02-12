#include "Fingerprint.h"
#define CMD_PACKET(packet,...)												\
	uint8_t data[] = {__VA_ARGS__};									\
	packet = StructurePacket(data, sizeof(data));

Fingerprint_Packet spacket = {
    .start_code = FINGERPRINT_STARTCODE,
    .address = 0xFFFFFFFF,
    .type = FINGERPRINT_COMMANDPACKET,
};

Fingerprint_Packet StructurePacket(uint8_t *data, uint16_t length)
{
	Fingerprint_Packet packet;
	packet = spacket;
	
	//length
	uint16_t length_packet = length + 2;
	packet.length = length_packet;
	
	uint16_t sum = (length_packet >> 8) + (length_packet & 0xFF) + packet.type;
	
	for(int i=0; i<4; i++)
	{
		packet.data[i] = data[0];
		sum += *data;
		data++;
	}
	packet.checksum = sum;
	return packet;
}
void sendPacket(Fingerprint_Packet packet)
{
	//send start code
	uint8_t value;
	value = (packet.start_code) >> 8;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	value = (packet.start_code) & 0xFF;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	//send address
	for(int i=0; i<4; i++)
	{
		value = 0xff;
		HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	}
	//send package identifier
	value = packet.type;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	//send package length
	value = packet.length << 8;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	value = (packet.length) & 0xFF;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	
	//send data
	for(int i =0; i < packet.length - 2; i++)
	{
		value = packet.data[i];
		HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	}
	//send checksum
	value = packet.checksum >> 8;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
	value = (packet.checksum) & 0xFF;
	HAL_UART_Transmit(&UART_HANDLER, &value, 1, 1000);
}

Fingerprint_Packet getPacket(Fingerprint_Packet spacket)
{
	Fingerprint_Packet rPacket;
//	uint8_t rdata[100];
	sendPacket(spacket);
	HAL_UART_Receive(&UART_HANDLER, rdata, 100, 1000);
	rPacket.start_code = (rdata[0] << 8) + rdata[1];
	rPacket.address = (rdata[2] << 24) + (rdata[3] << 16) + \
										(rdata[4] << 8) + rdata[5];
	rPacket.type = rdata[6];
	rPacket.length = (rdata[7] << 8) + rdata[8];
	for(int i=0; i<rPacket.length - 2; i++)
	{
		rPacket.data[i] = rdata[i + 9];
	}
	return rPacket;
}

bool verifyPassword()
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet,FINGERPRINT_VERIFYPASSWORD,(uint8_t)(PASSWORD >> 24), (uint8_t)(PASSWORD >> 16),
								(uint8_t)(PASSWORD >> 8), (uint8_t)(PASSWORD & 0xFF)); 
	//send cmd and return packet received
	rpacket = getPacket(packet);
	rpacket = getPacket(packet);
	return (rpacket.data[0] == FINGERPRINT_OK) ? true : false;
}

void LIB_verifyPassword()
{
	if(verifyPassword())
	{
		lcd_goto_XY(1,4);
		lcd_send_string("Found fingerprint sensor!");
	}
	else
	{
		lcd_send_string("Did not find fingerprint sensor!");
	}
}

uint8_t getImg()
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet,FINGERPRINT_GETIMAGE);
	rpacket = getPacket(packet);
	return rpacket.data[0];
}
/*
Generate fingerprint fearures from the original image in ImageBuffer
and restore in CharBuffer1 or CharBuffer2
slot: CharBuffer1 or CharBuffer2
*/
uint8_t Img2Tz(uint8_t slot)
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet, FINGERPRINT_IMAGE2TZ, slot);
	rpacket = getPacket(packet);
	return rpacket.data[0];
}
/*
Merge the feature file in CharBuffer1 and CharBuffer2 to generate a template
*/
uint8_t RegModel()
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet, FINGERPRINT_REGMODEL);
	rpacket = getPacket(packet);
	return rpacket.data[0];
}
/*
@brief Store template data in the specified feature buffer (CharBuffer1 or CharBuffer2)
				to flash Specify the location in the fingerprint library.
@param PageID: fingerprint library location number
*/
uint8_t StoreModel(uint8_t slot, uint16_t PageID)
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet,FINGERPRINT_STORE, (uint8_t)slot, 
						(uint8_t)(PageID >> 8), (uint8_t)(PageID & 0xff));
	rpacket = getPacket(packet);
	return rpacket.data[0];
}
/*
@brief Read the fingerprint template with the speccified ID Number in the flash database
			 into the template buffer CharBuffer1 or CharBuffer2
*/
uint8_t LoadModel(uint8_t slot, uint16_t PageID)
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet, FINGERPRINT_LOAD, (uint8_t)slot, 
						(uint8_t)(PageID >> 8), (uint8_t)(PageID & 0xff));
	rpacket = getPacket(packet);
	return rpacket.data[0];
}

uint8_t fingerprintSearch(uint8_t slot, uint16_t* PageID)
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet, FINGERPRINT_SEARCH, (uint8_t)slot, 0x00, 0x00,
						(uint8_t)(capacity >> 8), (uint8_t)(capacity & 0xff));
	rpacket = getPacket(packet);
	*PageID = (rpacket.data[1] << 8) + rpacket.data[2];
	return rpacket.data[0];
}

uint8_t deleteModel(uint16_t PageID)
{
	Fingerprint_Packet packet, rpacket;
	CMD_PACKET(packet, FINGERPRINT_DELETE, (uint8_t)(PageID >> 8), (uint8_t)(PageID & 0xff), 0x00, 0x01);
	rpacket = getPacket(packet);
	return rpacket.data[0];
}

uint8_t LIB_enrollFingerprint()
{
	setup_send("Ready to enroll ", "a fingerprint!");
	setup_send("Please type in ", "the ID #(1-127)");
	int id = readnumber();
	char str[10];
	sprintf(str, "%d", id);
	setup_send("Enrolling ID #",str);
	setup_send("Waiting finger", "to enroll as #");
	uint8_t p = -1;
	while(p != FINGERPRINT_OK)
	{
		p = getImg();
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image taken","");
				break;
			case FINGERPRINT_NOFINGER:
				setup_send("...............","");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				break;
			case FINGERPRINT_IMAGEFAIL:
				setup_send("Imaging error","");
				break;
			default:
				setup_send("Unknown error","");
				break;
		}
	}
		p = Img2Tz(1);
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image converted","");
				break;
			case FINGERPRINT_IMAGEMESS:
				setup_send("Image too messy","");
				return p;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_FEATUREFAIL:
				setup_send("Could not find","finger features");
				return p;
			case FINGERPRINT_INVALIDIMAGE:
				setup_send("Could not find","finger features");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
		setup_send("Remove finger","");
		HAL_Delay(2000);
		p = 0;
		while(p != FINGERPRINT_NOFINGER)
		{
			p = getImg();
		}
		setup_send("ID ",str);
		p = -1;
		setup_send("Place same", "finger again");
	while(p != FINGERPRINT_OK)
	{
		p = getImg();
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image taken","");
				break;
			case FINGERPRINT_NOFINGER:
				setup_send("...............","");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				break;
			case FINGERPRINT_IMAGEFAIL:
				setup_send("Imaging error","");
				break;
			default:
				setup_send("Unknown error","");
				break;
		}
	}
		p = Img2Tz(1);
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image converted","");
				break;
			case FINGERPRINT_IMAGEMESS:
				setup_send("Image too messy","");
				return p;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_FEATUREFAIL:
				setup_send("Could not find","finger features");
				return p;
			case FINGERPRINT_INVALIDIMAGE:
				setup_send("Could not find","finger features");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
		char str1[10];
		sprintf(str1, "for #%d", id);
		setup_send("Creating model",str1);
		
		p = RegModel();
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Prints matched!","");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_ENROLLMISMATCH:
				setup_send("Fingerprints","did not match");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
		p = StoreModel(1, id);
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Stored!","");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_BADLOCATION:
				setup_send("Could not store","in that location");
				return p;
			case FINGERPRINT_FLASHERR:
				setup_send("Error writing","to flash");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
	return true;
	
}
int readnumber()
{
	while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == 1 || HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 1){
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == 0)
		{
			return 1;
		}
		else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 0)
		{
			return 2;
		}
	}
	return 0;
}

uint8_t LIB_checkFingerprint()
{
	uint8_t p = getImg();
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image taken","");
				break;
			case FINGERPRINT_NOFINGER:
				setup_send("No finger   ","detected");
				return p;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_IMAGEFAIL:
				setup_send("Imaging error","");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
		p = Img2Tz(1);
		switch(p){
			case FINGERPRINT_OK:
				setup_send("Image converted","");
				break;
			case FINGERPRINT_IMAGEMESS:
				setup_send("Image too messy","");
				return p;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("Communication", "     error");
				return p;
			case FINGERPRINT_FEATUREFAIL:
				setup_send("Could not find","finger features");
				return p;
			case FINGERPRINT_INVALIDIMAGE:
				setup_send("Could not find","finger features");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
		uint16_t pageID;
		p = fingerprintSearch(1, &pageID);
		switch(p){
			case FINGERPRINT_OK:
				setup_send("    Found a ","   print match!");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				setup_send("    Communication", "     error");
				return p;
			case FINGERPRINT_ENROLLMISMATCH:
				setup_send("    Fingerprints","did not match");
				return p;
			default:
				setup_send("Unknown error","");
				return p;
		}
	  char str[10];
		sprintf(str,"%d",pageID);
		setup_send("Found ID #",str);
		return pageID;
}
uint8_t LIB_deleteFingerprint(int id)
{
	setup_send("Please type ID","you want to dlt");
	uint8_t p =-1;
	p = deleteModel(id);
	if(p == FINGERPRINT_OK)
	{
		setup_send("Deleted!","");
	}else if(p == FINGERPRINT_PACKETRECIEVEERR){
		setup_send("    Communication", "     error");
	}else if(p == FINGERPRINT_BADLOCATION){
		setup_send("Could not delete", "in that location");
	}else if(p == FINGERPRINT_FLASHERR){
		setup_send("Error writing", "to flash");
	}else{
		setup_send("Unknown error", "");
	}
	return p;
}
