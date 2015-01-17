#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// ------------------------------------------------------------------
// Configuration

// The mac address is the physical address associated with the 
// Ethernet port on the your device. It should be globally unique if
// the board is connected to a public network, or at least locally 
// unique if the board is connected to a private network. 
byte MacAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// The IP address is the internet protocol address for the board. Again
// it should be globally unique if connected to a public network or
// locally unique if connected to a private network. To communicate 
// successfully, it should also be on the same sub-net as your PC. 
// In practice that means the first three numbers should be the same
// as your PC's IP address. And the last number should be different to
// all devices on your network. Use ipconfig to find out your PC's IP address
IPAddress MyIPAddress(192, 168, 15, 177);
unsigned int LocalPort = 8888;


// The destination address is the IP address of the computer you want to send 
// messages to. It would normally be on the same sub-net as the Arduino. In practice
// this means the first three numbers of MyIPAddress and DestinationAddress should
// be the same. 
IPAddress DestinationAddress(192, 168, 15, 101);
unsigned int DestinationPort = 8888;

// ------------------------------------------------------------------
// Communications
EthernetUDP Udp; // To send & receive packets using UDP

char PacketBuffer[UDP_TX_PACKET_MAX_SIZE]; // Space to hold messages we receive.

unsigned long LastSend = 0; // Last time we sent a message
const long SendPeriod = 1000; // Time between sending messages [milliseconds]

// ------------------------------------------------------------------
// Data
bool EnableADCSend[] = { true, false, false, false, false, false, false, false };

// ------------------------------------------------------------------
// Functions
void ProcessUDPTraffic()
{
  // See if there is any data available and read it. 
  int nPacketSize = Udp.parsePacket();
  if (nPacketSize > 0)
  {
    int nRead = Udp.read(PacketBuffer, sizeof(PacketBuffer));
    if (nRead >= 1)
    {
      int nPort = PacketBuffer[0] - '0';
      if (nPort >= 0 && nPort < sizeof(EnableADCSend))
      {
        // Toggle sending data from the specified port. 
        EnableADCSend[nPort] = !EnableADCSend[nPort];
        
        Serial.print("Sending adc data for channel ");
        Serial.print(nPort);
        Serial.print(" is ");
        Serial.println(EnableADCSend[nPort] ? "on" : "off");
      }
    }
  } 
}

void SendADCData()
{
  if (millis() - LastSend < SendPeriod)
    return;
    
  LastSend = millis();
  
  for(int iPort = 0; iPort < sizeof(EnableADCSend); ++iPort)
  {
    if (EnableADCSend[iPort])
    {
      int nValue = analogRead(iPort);
      Udp.beginPacket(DestinationAddress, DestinationPort);
      Udp.print("{TIMEPLOT|data|ADC ");
      Udp.print(iPort);
      Udp.print("|T|");
      Udp.print(nValue);
      Udp.println('}');
      Udp.endPacket();
    }
  }
}

// ------------------------------------------------------------------
// Setup
void setup()
{
  // Start ethernet and udp
  Ethernet.begin(MacAddress, MyIPAddress);
  Udp.begin(LocalPort);
  
  // Start RS232 comms
  Serial.begin(9600);
}

// ------------------------------------------------------------------
// Loop
void loop()
{
  ProcessUDPTraffic();
  SendADCData();
}

