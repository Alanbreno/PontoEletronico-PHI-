#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <WakeOnLan.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

File myFile;

const char* ssid     = "PHI";
const char* password = "PontoEletronico";

boolean wifiConnected = false;

SoftwareSerial mySerial(D1, D2);

WiFiUDP UDP;
/**
   The Magic Packet needs to be sent as BROADCAST in the LAN
*/
IPAddress computer_ip(255, 255, 255, 255);

/**-
   The targets MAC address to send the packet to
*/
byte mac[] = { 0x10, 0x7B, 0x44, 0x3A, 0x26, 0x9B };


boolean connectWifi(); //empty methods defined here, for declaration see lower.
void sendWOL();

void setup() {
  // Initialise Serial connection
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.print("Inicializando o cartão SD...");

  if (!SD.begin(D8)) {
    Serial.println("Falha na inicialização!");
    while (1);
  }
  Serial.println("Inicialização concluída.");


  // Initialise wifi connection
  wifiConnected = connectWifi();

  UDP.begin(9); //start UDP client, not sure if really necessary.
}

void loop()
{
  String comandoLido = leComandoMySerial();
  String comandoSerial = leComandoSerial();

  if (comandoLido == "SendMagicPacket") {
    WakeOnLan::sendWOL(computer_ip, UDP, mac, sizeof mac);
    WakeOnLan::sendWOL(computer_ip, UDP, mac, sizeof mac);
    WakeOnLan::sendWOL(computer_ip, UDP, mac, sizeof mac);
    mySerial.println("<MagicPacketEnviado>");

  }

  if (comandoLido == "ReadyToWrite") {

    String data = "";
    while (data == "") {
      data = leComandoMySerial();
    }
    //Serial.println(data);
    String url = data.substring(data.indexOf(" ") + 1, data.lastIndexOf(" "));
    //Serial.println(url);
    //Serial.println(url.substring(0, url.lastIndexOf("/")));
    if (!SD.exists(url.substring(0, url.lastIndexOf("/")))) {
      SD.mkdir(url.substring(0, url.lastIndexOf("/")));
    }
    url.concat(".txt");
    //Serial.println(url);
    myFile = SD.open(url, FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
      Serial.print("Escrevendo no TXT...");
      myFile.println(data);
      // close the file:
      myFile.close();
      Serial.println("Pronto.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("Erro ao abrir txt");
    }

  }

  if ( comandoSerial == "LerTXT" ) {

    String data = "";
    while (data == "") {
      data = leComandoSerial();
    }
    data.concat(".txt");
    File dataFile = SD.open(data);

    // if the file is available, write to it:
    if (dataFile) {
      while (dataFile.available()) {
        Serial.write(dataFile.read());
      }
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.print("Erro ao abrir o arquivo ");
      Serial.println(data);
    }
  }

  delay(10);

}

// connect to wifi – returns true if successful or false if not
boolean connectWifi() {
  boolean state = true;
  int i = 0;
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Conectando ao WiFi");

  // Wait for connection
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Conectado ao ");
    Serial.println(ssid);
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Falha na conexão.");
  }
  return state;
}

String leComandoMySerial() {
  String comando = "";                  //string que guarda o comando transmitido
  char caractere;                       //char que guarda o byte lido atualmente
  char startComando = '<';              //caractere que antecede o comando
  char endComando = '>';                //caractere que finaliza o comando
  boolean salvarDados = false;

  while (mySerial.available() > 0) {
    //ler o byte disponivel na serial
    caractere = mySerial.read();

    //verifica se é igual ao byte de inicio de comando
    if (caractere == startComando) {
      delay(10);
      salvarDados = true;
      continue;                        //pula para proxima iteração para ler um caractere válido
      //verifica se é igual ao byte de fim de comando
    } else if (caractere == endComando) {
      salvarDados = false;
      return comando;
    }

    if (salvarDados) {
      comando.concat(caractere);
    }
    delay(10);
  }
}
String leComandoSerial() {
  String comando = "";                  //string que guarda o comando transmitido
  char caractere;                       //char que guarda o byte lido atualmente
  char startComando = '<';              //caractere que antecede o comando
  char endComando = '>';                //caractere que finaliza o comando
  boolean salvarDados = false;

  while (Serial.available() > 0) {
    //ler o byte disponivel na serial
    caractere = Serial.read();

    //verifica se é igual ao byte de inicio de comando
    if (caractere == startComando) {
      delay(10);
      salvarDados = true;
      continue;                        //pula para proxima iteração para ler um caractere válido
      //verifica se é igual ao byte de fim de comando
    } else if (caractere == endComando) {
      salvarDados = false;
      return comando;
    }

    if (salvarDados) {
      comando.concat(caractere);
    }
    delay(10);
  }
}
