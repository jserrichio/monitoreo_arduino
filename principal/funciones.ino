


float leerTemp(){
    // Lee el valor desde el sensor
    tempC = analogRead(tempPin);
    // Convierte el valor a temperatura
    tempC = (5.0 * tempC * 100.0)/1024.0;
    return tempC;
}

void calcularTemperatura(){
    float temp_actual = leerTemp();
    cant_temperaturas++;
    Serial.print("Cantidad de temperaturas acumuladas= ");
    Serial.println(cant_temperaturas);
    temp_acumuladas = temp_acumuladas + temp_actual;
    Serial.print("temperaturas acumuladas= ");
    Serial.println(cant_temperaturas);

    if (cant_temperaturas==5) {
      float promedio = temp_acumuladas / 5;
      enviarDatos(promedio);
      cant_temperaturas=0;
      temp_acumuladas=0;
    }
}

unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:

  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

void actualizarRelojNTP(){

  Serial.print("Configurando NTP con el servidor ");
  Serial.println(timeServer);
  sendNTPpacket(timeServer);  // Enviar unaa peticion al servidor NTP
  delay(1000);                  // Damos tiempo a la respuesta
  if ( Udp.parsePacket() ) {
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // Leemos el paquete
    // La hora empieza en el byte 40 de la respuesta y es de 4 bytes o 2 words
    // Hay que extraer ambas word:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Segundos desde 1 Enero de 1900 = " );
    Serial.println(secsSince1900);
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    setTime(epoch);
    // Lo pongo en zona horaria -3
    adjustTime(-10800);
    Serial.print("Booted at = ");
    imprimirHora();
    }
  else {
    Serial.println("Error de red");
    delay(1000);
  }
}

void imprimirHora(){
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    if (second()<10) {
      Serial.print("0");
    }
    Serial.println(second());
}

void cron(){
  //Una vez por minuto minutos
  if (second()==0){
    //temporizador++;
    imprimirHora();
    //calcularTemperatura;
    int temperatura = leerTemp();
    Serial.println("temperatura actual");
    Serial.println(temperatura);


    int temp_int = temperatura;
    if (0,5 < (temperatura - temp_int)){
          temp_int++;
    }
    enviarDatos(temp_int);
  }
    //if (temporizador==10){
    //  enviarDatos();
     // temporizador=0;
    //}
    //Una vez por hora
    //if ((minute()==0)&&(second()==0)){
    //
    //}
    //Una vez por dia
  if ((minute()==0)&&(second()==0)&&(hour()==0)){
    actualizarRelojNTP();
  }
}

void enviarDatos(float temperatura){
  // Proceso de envio de muestras al servidor
  Serial.println("Connecting...");
  if (client.connect(servidor_datos, 80)>0) {  // Conexion con el servidor

    client.print("GET /monitoreo/index.php/Cargardatos?valor="); // Enviamos los datos por GET
    client.print(temperatura);
    client.println(" HTTP/1.0");
    client.println("User-Agent: Arduino 1.0");
    client.println();
    Serial.println("Conectado");
    Serial.print("Dato enviado: ");
    Serial.println(temperatura);
  } else {
    Serial.println("Fallo en la conexion");
  }
    if (!client.connected()) {
      Serial.println("Disconnected!");
  }
  client.stop();
  client.flush();
}

void serverWeb(){
   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          client.println("<h1>");
          client.println("hola!!!");
          client.println("</h1>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
