/*
 *  Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
   
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version:		3.0
 *  Design:			David Gascón
 *  Implementation:	Yuri Carmona
 */


#ifndef __WPROGRAM_H__
	#include <WaspClasses.h>
#endif

#include "WaspXBee900HP.h"

/******************************************************************************
 * AT COMMANDS (FLASH Definitions)
 ******************************************************************************/

/// table_900HP /////////////////////////////////////////////////////////////////

const char	get_RF_errors_900[]		PROGMEM	 = 	"7E0004085245520E"; 	// AT+ER
const char 	get_good_pack_900[]		PROGMEM  = 	"7E0004085247441A";		// AT+GD
const char 	get_trans_errors_900[]	PROGMEM  = 	"7E000408525452FF";		// AT+TR
const char 	get_availablefreq_900[] PROGMEM  = 	"7E0004085241461E";		// AT+AF
const char	set_channelmask_900[] 	PROGMEM = 	"7E000C0852434D000000000000000000"; // AT+CM
const char	get_channelmask_900[] 	PROGMEM = 	"7E00040852434D15"; 	// AT+CM
const char	get_minFreqCount_900[] 	PROGMEM = 	"7E000408524D4612"; 	// AT+MF
const char	set_preambleid_900[] 	PROGMEM = 	"7E0005085248500000"; 	// AT+HP
const char	get_preambleid_900[] 	PROGMEM = 	"7E0004085248500D"; 	// AT+HP
const char	set_to_900[] 			PROGMEM = 	"7E00050852544F0000"; 	// AT+TO
const char	get_to_900[] 			PROGMEM = 	"7E00040852544F02"; 	// AT+TO
const char 	set_network_hops_DM[]	PROGMEM  = 	"7E000508524E480000";	// AT+NH
const char 	get_network_hops_DM[]	PROGMEM  = 	"7E000408524E480F";		// AT+NH
const char 	set_network_delay_DM[]	PROGMEM  = 	"7E000508524E4E0000";	// AT+NN
const char 	get_network_delay_DM[]	PROGMEM  = 	"7E000408524E4E09";		// AT+NN
const char 	set_network_retries_DM[] PROGMEM = 	"7E000508524D520000";	// AT+MR
const char 	get_network_retries_DM[] PROGMEM = 	"7E000408524D5206";		// AT+MR
const char 	set_retries_DM[] 		PROGMEM  = 	"7E0005085252520000"; 	// AT+RR  
const char 	get_retries_DM[] 		PROGMEM  = 	"7E00040852525201";		// AT+RR


const char* const table_900HP[] PROGMEM= 	  
{   
	get_RF_errors_900,		// 0
  	get_good_pack_900,		// 1
  	get_trans_errors_900,	// 2 
  	get_availablefreq_900,	// 3
  	set_channelmask_900,	// 4
  	get_channelmask_900,	// 5
  	get_minFreqCount_900,	// 6
  	set_preambleid_900,		// 7
  	get_preambleid_900,		// 8
  	set_to_900,				// 9
  	get_to_900,				// 10
  	set_network_hops_DM,	// 11
  	get_network_hops_DM,	// 12  	
  	set_network_delay_DM,	// 13
  	get_network_delay_DM,	// 14
  	set_network_retries_DM,	// 15
  	get_network_retries_DM,	// 16
  	set_retries_DM,			// 17
  	get_retries_DM,			// 18
  	
};


/******************************************************************************
 * Class methods
 ******************************************************************************/

/*
 * Function: It initalizes all the necessary variables including its parent's 
 * variables
 * 
 * Parameters:
 * 'protocol_used' : specifies the protocol used in the XBee module 
 * 'uart_used' : specifies the UART where the data are sent to (SOCKET0 or SOCKET1)
 * Returns: void
*/
void WaspXBee900HP::init( uint8_t uart_used )
{	
	protocol = XBEE_900HP;
	uart = uart_used;
	
	// in the case the XBee is plugged to SOCKET0 it is necessary to make sure
	// that the multiplexer is selecting the XBee module
	if( uart_used == SOCKET0)
	{
		Utils.setMuxSocket0();
	}
	
	// init class variables
	data_length = 0;
	it = 0;
	rxFrameType = 0;	
	pos=0;
	discoveryOptions = 0x00;
	
	scanTime[0] = SCAN_TIME_DIGIMESH_H;
	scanTime[1] = SCAN_TIME_DIGIMESH_L;
	encryptMode = 0;
	timeRSSI = TIME_RSSI_DIGIMESH;

	frameNext = 0;
	replacementPolicy = XBEE_OUT;	
	
	// init flags
	error_AT = 2;
	error_RX = 2;
	error_TX = 2;
	
	// clear buffers
	clearFinishArray();
	clearCommand();
}

/*
 Function: Read the number of times the RF receiver detected a CRC or length error
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "_errorsRF" variable number of times CRC or length error
*/
uint8_t WaspXBee900HP::getRFerrors()
{
	int8_t error = 2;     
    error_AT = 2;    
    char buffer[20];
    
    // get_RF_errors_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[0])));
    
    gen_data(buffer);
    error=gen_send(buffer);
    
    if(!error)
    {
		// copy data[0-1] to _errorsRF
		memcpy( &_errorsRF, data, 2 );           
    } 
    return error;
}

/*
 Function: Read the number of good frames with valid MAC headers that are 
 received on the RF interface
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "_goodPackets" variable the number of good frames received
*/
uint8_t WaspXBee900HP::getGoodPackets()
{
    int8_t error = 2;     
    error_AT = 2;    
    char buffer[20];
    
    // get_good_pack_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[1])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if(!error)
    {
		// copy data[0-1] to _goodPackets
		memcpy( &_goodPackets, data, 2 ); 
    } 
    return error;
}


/*
 Function:  Read the number of MAC frames that exhaust MAC retries without ever 
 receiving a MAC acknowledgement message from the adjacent node
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "_transmisionErrors" variable the number of MAC frames 
 that exhaust	 
*/
uint8_t WaspXBee900HP::getTransmisionErrors()
{
    int8_t error = 2;     
    error_AT = 2;    
    char buffer[20];
    
    //get_trans_errors_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[2])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if(!error)
    {
		// copy data[0-1] to _transmisionErrors
		memcpy( &_transmisionErrors, data, 2 );  
    } 
    return error;
}


/*
 * 
 * name: getAvailableFreq
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::getAvailableFreq()
{
    int8_t error = 2;     
    error_AT = 2;    
    char buffer[20];
    
    //get_availablefreq_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[3])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if(!error)
    {
		// module provides a 9-byte payload
		// the first byte is 0x00
		// the rest of the bytes are the contents of the command answer:
		_availableFreq[0] = data[1];
		_availableFreq[1] = data[2];
		_availableFreq[2] = data[3];
		_availableFreq[3] = data[4];
		_availableFreq[4] = data[5];
		_availableFreq[5] = data[6];
		_availableFreq[6] = data[7];
		_availableFreq[7] = data[8];
    } 
    return error;
}




/*
 * 
 * name: setChannelMask
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::setChannelMask( uint8_t* mask )
{
	int8_t error=2;     
    error_AT=2;
    char buffer[50];
    
    // set_channelmask_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[4])));

    gen_data(buffer, mask);
    gen_checksum(buffer); 
    error = gen_send(buffer);
  
    
    if( error == 0 )
    {     
		_channelMask[0] = mask[0];
		_channelMask[1] = mask[1];
		_channelMask[2] = mask[2];
		_channelMask[3] = mask[3];
		_channelMask[4] = mask[4];
		_channelMask[5] = mask[5];
		_channelMask[6] = mask[6];
		_channelMask[7] = mask[7];
    }
    return error;    
}


/*
 * 
 * name: getChannelMask
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::getChannelMask()
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // get_channelmask_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[5])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		// module provides a 9-byte payload
		// the first byte is 0x00
		// the rest of the bytes are the contents of the command answer:
		_channelMask[0] = data[1];
		_channelMask[1] = data[2];
		_channelMask[2] = data[3];
		_channelMask[3] = data[4];
		_channelMask[4] = data[5];
		_channelMask[5] = data[6];
		_channelMask[6] = data[7];
		_channelMask[7] = data[8];
    }
    return error;    
}





/*
 * 
 * name: getMinFreqCount
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::getMinFreqCount()
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // get_minFreqCount_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[6])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		_minFreqCount = data[0];	
    }
    return error;    
}





/*
 * 
 * name: setPreambleID
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::setPreambleID( uint8_t preambleID )
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // set_preambleid_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[7])));
    
    gen_data(buffer, preambleID);
    gen_checksum(buffer); 
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		_preambleID = preambleID;
    }
    return error;    
}


/*
 * 
 * name: getPreambleID
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::getPreambleID()
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // get_preambleid_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[8])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		_preambleID = data[0];	
    }
    return error;    
}



/*
 * 
 * name: setTransmitOptions
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::setTransmitOptions( uint8_t newTransmitOption )
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // set_to_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[9])));
    
    gen_data(buffer, newTransmitOption);
    gen_checksum(buffer); 
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		_transmitOptions = newTransmitOption;	
    }
    return error;    
}


/*
 * 
 * name: getTransmitOptions
 * @param void
 * @return '0' if ok, '1' otherwise
 * 
 */
uint8_t WaspXBee900HP::getTransmitOptions()
{
	int8_t error=2;     
    error_AT=2;
    char buffer[20];
    
    // get_to_900
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[10])));
    
    gen_data(buffer);
    error = gen_send(buffer);
    
    if( error == 0 )
    {
		_transmitOptions = data[0];	
    }
    return error;    
}





/*
 * Function: Send a packet from one XBee to another XBee in API mode
 * 
 * Parameters: 
 *   packet :	A struct of packetXBee type
 * 
 * Returns: Integer that determines if there has been any error 
 * 	error=2 --> The command has not been executed
 * 	error=1 --> There has been an error while executing the command
 * 	error=0 --> The command has been executed with no errors
 * 
 * --> DIGI's XBee Packet inner structure:
 * 
 * StartDelimiter(1B) + Length(2B) +  Frame Data(variable) + Checksum(1B)
 *  ______________     ___________     __________________     __________
 * |              |   |     |     |   |					 |   |			|
 * |	0x7E	  | + | MSB | LSB | + |    Frame Data    | + |	1 Byte	|
 * |______________|   |_____|_____|   |__________________|   |__________|
 * 
*/
uint8_t WaspXBee900HP::sendXBeePriv(struct packetXBee* packet)
{
	// Local variables
	uint8_t TX[400];
    uint16_t counter = 0;
    uint8_t protegido = 0;
    uint8_t tipo = 0;
    int8_t error = 2; 
    
    clearCommand();
    
    // clear TX variable where the frame is going to be filled
	memset( TX, 0x00, sizeof(TX) );
    
    /* Create the XBee frame */
    TX[0] = 0x7E;
    TX[1] = 0x00;
    
    // set frame ID as 0x01, so response message will be sent
    TX[4] = 0x01;
    
    // init variables
    it = 0;
    error_AT = 2;
    error_TX = 2;
    

    if( (packet->mode==BROADCAST) || (packet->mode==UNICAST) )
    {
		// set fragment length for 'Transmit Request' frames (0x10)
		TX[1] = ((14+packet->data_length)>>8)&0xFF;
        TX[2] = (14+packet->data_length)&0xFF;        
  
        TX[3] = 0x10; // frame Type
        tipo = 18;	// number of header bytes
        
        // set 64-Destination Address   
        if(packet->mode == BROADCAST)
        {
			// set BROADCAST address
            TX[5] = 0x00;
			TX[6] = 0x00;
			TX[7] = 0x00;
			TX[8] = 0x00;
			TX[9] = 0x00;
			TX[10] = 0x00;
			TX[11] = 0xFF;
			TX[12] = 0xFF;	                
        }
        else if(packet->mode==UNICAST)
        {
			// set chosen address in setDestinationParams function
			TX[5] = packet->macDH[0];
			TX[6] = packet->macDH[1];
			TX[7] = packet->macDH[2];
			TX[8] = packet->macDH[3];
			TX[9] = packet->macDL[0];
			TX[10] = packet->macDL[1];
			TX[11] = packet->macDL[2];
			TX[12] = packet->macDL[3];       
        }
        
        // set frame bytes
        TX[13] = 0xFF;
        TX[14] = 0xFE; 
		TX[15] = 0x00;	// if set to 0, the BH parameter will be set
        TX[16] = 0x00;  // if the this field is 0, then the TO parameter will be used
        it = 0;
        
        // generate RF Data payload which is composed by [Api header]+[Data]
        genDataPayload(packet,TX,17);
        
        // set checksum     
        TX[packet->data_length+17] = getChecksum(TX); 
    }
    else // CLUSTER Type (Explicit Addressing Command Frame)
    {
		// set fragment length for 'Explicit Addressing Command' frames (0x11)
		TX[1] = ((20+packet->data_length)>>8)&0xFF;
        TX[2] = (20+packet->data_length)&0xFF;
        
        // set frame ID
        TX[3] = 0x11; 
        
        tipo = 24;	// number of header bytes
		TX[5] = packet->macDH[0];
		TX[6] = packet->macDH[1];
		TX[7] = packet->macDH[2];
		TX[8] = packet->macDH[3];
		TX[9] = packet->macDL[0];
		TX[10] = packet->macDL[1];
		TX[11] = packet->macDL[2];
		TX[12] = packet->macDL[3];     
	
        TX[13] = 0xFF;
        TX[14] = 0xFE;        
        TX[15] = packet->SD;
        TX[16] = packet->DE;
        TX[17] = 0x00;
        TX[18] = packet->CID[0];     
        TX[19] = packet->PID[0];
        TX[20] = packet->PID[1];     
        TX[21] = 0x00;
        TX[22] = 0x00;
        it = 0;
        
        // generate RF Data payload which is composed by [Api header]+[Data]
        genDataPayload(packet,TX,23);

        // set checksum
        TX[packet->data_length+23] = getChecksum(TX);             
    }
    
    // Generate the escaped API frame (it is necessary because AP=2)
    gen_frame_ap2( packet, TX, protegido, tipo);
        
    // switch MUX in case SOCKET1 is used
	if( uart==SOCKET1 )
	{
		Utils.setMuxSocket1();
	}
	
	// send frame through correspondent UART
    counter = 0;
    while( counter < ( packet->data_length + tipo + protegido ) )
    {
		// print byte through correspondent UART
		printByte( TX[counter], uart); 
        counter++;
    }
        
    counter = 0;    
   
	// read XBee's answer to TX request
	error_TX = txZBStatusResponse();
	error = error_TX; 
	
    packet->deliv_status = delivery_status;
    packet->discov_status = discovery_status;
    packet->true_naD[0] = true_naD[0];
    packet->true_naD[1] = true_naD[1];
    packet->retries = retries_sending;
    
    return error;
}



////////////////////////////////////////////////////////////////////////////////
// DigiMesh methods
////////////////////////////////////////////////////////////////////////////////


/*
 Function: Set the maximum number of hops expected to be seen in a network route
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Change the NH command
 Parameters:
   nhops: maximum number of hops (1-0xFF)
*/
uint8_t WaspXBee900HP::setNetworkHops(uint8_t nhops)
{
    int8_t error=2;
	error_AT=2;
	char buffer[20];
	  
	// set_network_hops_DM
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[11])));
    
	gen_data(buffer,nhops);
	gen_checksum(buffer);
	error=gen_send(buffer);

    if(!error)
    {
        _networkHops = nhops;
    }
    return error;
}

/*
 Function: Read the maximum number of hops expected to be seen in a network route
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "networkHops" variable max number of hops
*/
uint8_t WaspXBee900HP::getNetworkHops()
{
    int8_t error=2;        
	error_AT=2;
	char buffer[20];	    
	
	// get_network_hops_DM
	strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[12])));
	gen_data(buffer);
	error=gen_send(buffer);
    
    if(error==0)
    {
        _networkHops = data[0];
    } 
    return error;
}



/*
 Function: Set the maximum random number of network delay slots before 
 rebroadcasting a network packet
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Change the NN command
 Parameters:
   dslots: maximum number of delay slots (0-0x0A)
*/
uint8_t WaspXBee900HP::setNetworkDelaySlots(uint8_t dslots)
{
    int8_t error=2; 
	error_AT=2;
	char buffer[20];	    
	
	// set_network_delay_DM
	strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[13])));
	gen_data(buffer,dslots);
	gen_checksum(buffer);
	error=gen_send(buffer);
    
    if(error==0)
    {
        _netDelaySlots = dslots;
    }
    return error;
}

/*
 Function: Read the maximum random number of network delay slots before 
 rebroadcasting a network packet
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "netDelaySlots" variable max random number of net 
 delay slots
*/
uint8_t WaspXBee900HP::getNetworkDelaySlots()
{
    int8_t error=2;
	error_AT=2;
	char buffer[20];	    
	
	// get_network_delay_DM
	strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[14])));
	gen_data(buffer);
	error=gen_send(buffer);
    
    if(error==0)
    {
        _netDelaySlots = data[0];
    } 
    return error;
}



/*
 Function: Set the maximum number of network packet delivery attempts
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Change the MR command
 Parameters:
   mesh: maximum number of attempts (0-7)
*/
uint8_t WaspXBee900HP::setMeshNetworkRetries(uint8_t mesh)
{
    int8_t error=2; 
	error_AT=2;
	char buffer[20];	    
	
	// set_network_retries_DM
	strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[15])));
	gen_data(buffer,mesh);
	gen_checksum(buffer);
	error=gen_send(buffer);
    
    if(error==0)
    {
        _meshNetRetries = mesh;
    }
    return error;
}

/*
 Function: Read the maximum number of network packet delivery attempts
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "meshNetRetries" variable max number of net packet 
 delivery attempts
*/
uint8_t WaspXBee900HP::getMeshNetworkRetries()
{
    int8_t error=2;
	error_AT=2;
	char buffer[20];	    
	
	// get_network_retries_DM
	strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[16])));
	gen_data(buffer);
	error=gen_send(buffer);
    
    if(error==0)
    {
        _meshNetRetries = data[0];
    } 
    return error;
}




/*	
 * setRetries
 * 
 * Set the maximum number of MAC level packet delivery attempts for unicasts. 
 * If RR is non-zero packets sent from the radio will request an acknowledgement, 
 * and can be resent up to RR times if no acknowledgements are received.
 * 
 * Returns: Integer that determines if there has been any error 
 * 	error=2 --> The command has not been executed
 * 	error=1 --> There has been an error while executing the command
 * 	error=0 --> The command has been executed with no errors
 * 
 * Values: Change the RR command
 * Parameters:
 * 	retry: number of retries (0x00-0x0F). Default=0x0A
*/
uint8_t WaspXBee900HP::setRetries(uint8_t retry)
{
    int8_t error = 2;
    error_AT = 2; 
 
	// set_retries_DM
    char buffer[20];
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[17])));
   
    gen_data(buffer,retry);
    gen_checksum(buffer);
    error = gen_send(buffer);

    if(!error)
    {
        _retries = retry;
    }
    return error;
}

/*
 Function: Get the retries that specifies the RR command
 Returns: Integer that determines if there has been any error 
   error=2 --> The command has not been executed
   error=1 --> There has been an error while executing the command
   error=0 --> The command has been executed with no errors
 Values: Stores in global "retries" variable the number of retries
*/
uint8_t WaspXBee900HP::getRetries()
{
    int8_t error = 2;
    char buffer[20];
    error_AT = 2;
    
    //get_retries_DM
    strcpy_P(buffer, (char*)pgm_read_word(&(table_900HP[18])));
    if( buffer == NULL ) return 1;

    gen_data(buffer);
    error = gen_send(buffer);
    

    if(!error)
    {
        _retries = data[0];
    }
    return error;
}





/// Pre-instantiate object
WaspXBee900HP	xbee900HP = WaspXBee900HP();
