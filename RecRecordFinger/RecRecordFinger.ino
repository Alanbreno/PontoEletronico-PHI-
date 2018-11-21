#include <Adafruit_Fingerprint.h>
#include <LCD5110_Basic.h>
#include <Wire.h>        //Biblioteca para manipulação do protocolo I2C
#include <DS3231.h>      //Biblioteca para manipulação do DS3231

DS3231 Clock;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
LCD5110 tela(8, 9, 10, 12, 11);
/*Cria objeto da classe LCD5110
  CLK – Pino 8
  DIN – Pino 9
  DC – Pino 10
  RST – Pino 12
  CE – Pino 11
*/

bool Century,h12,PM;

//Obtendo as fontes
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];



uint8_t id;

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  while (!Serial);
  delay(100);
  tela.InitLCD();   //Inicializando o display

  tela.setFont(SmallFont);
  tela.print("Inicializando.", LEFT, 0);
  Wire.begin();
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);


  if (finger.verifyPassword()) {
    tela.print("Pronto!", LEFT, 8);
    Serial.println("Found fingerprint sensor!");
  } else {
    tela.print("Erro", LEFT, 8);
    tela.print("Aperte Reset", LEFT, 16);

    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");

}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop()
{
  String RetornoComando = leComandoSerial();
  if (RetornoComando != "" && RetornoComando == "MagicPacketEnviado") {
    tela.clrScr();
    delay(300);
    String hora = String(String(Clock.getHour(h12, PM))+":"+String(Clock.getMinute())+":"+String(Clock.getSecond()));
    String ID = String("ID: "+String(finger.fingerID));
    tela.print("Bem-Vindo", CENTER, 0);
    tela.print(hora, CENTER, 20);
    tela.print(ID, CENTER, 40);
    Serial.println(RetornoComando);
  }
  if (Serial.available()) {
    Serial.readString();
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    id = readnumber();
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);

    while (!  getFingerprintEnroll() );
    Serial.readString();


  }
  getFingerprintIDez();
  delay(1);            //don't ned to run this at full speed.
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:

        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:

        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

}
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  Serial2.print("<SendMagicPacket>");
  Serial2.print("<ReadyToWrite>");
  Serial2.print("<");
  Serial2.print("ID-");
  Serial2.print(finger.fingerID);
  Serial2.print(" ");
  Serial2.print(Clock.getYear(), DEC);
  Serial2.print("/");
  Serial2.print(Clock.getMonth(Century), DEC);
  Serial2.print("/");
  Serial2.print(Clock.getDate(), DEC);
  Serial2.print(" ");
  Serial2.print(Clock.getHour(h12, PM), DEC); //24-hr
  Serial2.print(":");
  Serial2.print(Clock.getMinute(), DEC);
  Serial2.print(":");
  Serial2.print(Clock.getSecond(), DEC);
  Serial2.print(">");
  
  return finger.fingerID;
}

String leComandoSerial() {
  String comando = "";                  //string que guarda o comando transmitido
  char caractere;                       //char que guarda o byte lido atualmente
  char startComando = '<';              //caractere que antecede o comando
  char endComando = '>';                //caractere que finaliza o comando
  boolean salvarDados = false;

  while (Serial2.available() > 0) {
    //ler o byte disponivel na serial
    caractere = Serial2.read();

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
