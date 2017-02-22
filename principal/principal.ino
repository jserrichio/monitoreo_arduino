#include <SPI.h>

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <EthernetUdp.h>
#include <Time.h>


//parametros de configuracin: setup
//int frecuencia = 9600;
//int delay_un_segundo = 1000;


//Declaracion de placa ethernet
byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 } ;
EthernetClient client;

//Declaro UDP y NTP
unsigned int localPort = 80;      //  Puerto local para escuchar UDP
IPAddress timeServer(163,10,43,120);     // time.nist.gov NTP server (fallback)
const int NTP_PACKET_SIZE = 48;         // La hora son los primeros 48 bytes del mensaje
byte packetBuffer[ NTP_PACKET_SIZE];      // buffer para los paquetes
EthernetUDP Udp;                // Una instancia de UDP para enviar y recibir mensajes
unsigned long epoch;
int pantallaActual = 1;

//temperatura
float tempC;
int tempPin = 1; // Definimos la entrada en pin A0

//lo uso para calcular el promedio
int cant_temperaturas = 0;
float temp_acumuladas = 0;

byte servidor_datos[] = { 163,10,44,54 }; // Direccion IP del servidor donde enviar los datos

//timer para subir datos cada 15 min
int temporizador = 0;

//defino server http
EthernetServer server(80);


void setup() {
  Serial.begin(frecuencia);
  Serial.println("Configurando IP");
  while (Ethernet.begin(mac) == 0)    //Si devuelve error
     {
          Serial.println("No ha sido possible configurar la Ethernet por DHCP");
          delay (delay_un_segundo);
          //while (true; )    ;            // No sigas, quédate aquí eternamente
     }

  Serial.print("Mi direccion IP es: ");
  for (byte B = 0; B < 4; B++)
     {
         Serial.print(Ethernet.localIP()[B], DEC);
         Serial.print(".");
    }

  Serial.println();
  Udp.begin(localPort);
  actualizarRelojNTP();
  //iniciar servidor web
  server.begin();
 }

void loop(){
  // Imprimo al LCD
  cron();
  serverWeb();
  delay(delay_un_segundo);
}
