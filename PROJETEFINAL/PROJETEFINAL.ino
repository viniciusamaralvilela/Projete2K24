#include <SPI.h>
#include <MFRC522.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 4
#define SS_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth não está habilitado! Por favor, habilite no menu de configuração.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile para Bluetooth não está habilitado. Só está disponível para o chip ESP32.
#endif
BluetoothSerial SerialBT;
int tempo = 0;
int recarga = 0;

byte uidcartao1[] = { 0xD3, 0x93, 0x18, 0xF7 };
byte uidcartao2[] = { 0x63, 0x23, 0x05, 0xF7 };
byte uidcartao3[] = { 0x13, 0x67, 0x08, 0xF7 };

char buffer[34];
byte bufferw[16];
int saldo = 0;

MFRC522::MIFARE_Key key;
byte block;                  // Declaração do bloco
MFRC522::StatusCode status;  // Declaração do status

bool comparaUID(byte *uidLido, byte *uidSalvo, byte tamanho) {
  for (byte i = 0; i < tamanho; i++) {
    if (uidLido[i] != uidSalvo[i]) {
      return false;
    }
  }
  return true;
}

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define relay 25
void setup() {
  Serial.begin(115200);
  lcd.init();            // Inicializa o LCD
  lcd.backlight();       // Liga o backlight
  SPI.begin();
  mfrc522.PCD_Init();
  SerialBT.begin("ESP32_SPO");
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  delay(500);
  digitalWrite(relay, LOW);
  
}

void modo_recarga(void) {
  lcd.setCursor(0,0);
  lcd.print("  Modo recarga");
  Serial.println("Modo recarga");
  lcd.setCursor(0,1);
  Serial.println("Faca a recarga");
  delay(500);
  tempo = 10000;
  recarga = 0;
  while (tempo > 0) {
    if (SerialBT.available()) {
      String rx2 = SerialBT.readStringUntil('\n');
      if (rx2.toInt()) {
        recarga = rx2.toInt();
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Recarga no valor");
        lcd.setCursor(6,1);
        lcd.print(recarga);
        tempo = 1;
      }
    }
    tempo--;
    delay(1);
  }
  Serial.println("Aproxime cartao");
  tempo = 10000;
  saldo = -1;
  while(tempo>0 && saldo==-1) {
    verifica_cartao();
    tempo--;
    delay(1);
  }
  delay(100);
  if (saldo != -1) {
    saldo = saldo + recarga;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" novo saldo = ");
    lcd.print(saldo);
    delay(3000);
    lcd.clear();
    Serial.print(" novo saldo = ");
    Serial.print(saldo);
    sprintf((char *)bufferw, "%i", saldo);
    Serial.println((char *)bufferw);
    write(); 
  }
}

void loop() {
  //Serial.print("@");
  lcd.setCursor(0, 0);
  lcd.print("      SPO");
  lcd.setCursor(0,1);
  lcd.print("  Equipe 2207");
  delay(10);
  if (SerialBT.available()) {
    String rx = SerialBT.readStringUntil('\n');
    if (rx == "R") {
      Serial.println("Modo recarga");
      Serial.println("Aproxime o cartão");
      recarga = 0;
      modo_recarga();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    }else{
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    }
  }
  if (verifica_cartao()) {
    if (comparaUID(mfrc522.uid.uidByte, uidcartao1, mfrc522.uid.size)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("  TRABALHADOR");
      delay(1500);
      lcd.clear();
      Serial.println("cartao TRABALHADOR detectado");
      if (saldo != 0){
        saldo = saldo - 1;
      sprintf((char *)bufferw, "%i", saldo);
      lcd.setCursor(0,0);
      lcd.print("   saldo =");
      Serial.print("buffer = ");
      lcd.setCursor(11,0);
      lcd.println((char*)bufferw);
      Serial.println((char *)bufferw);
      write();
      digitalWrite(relay, HIGH);
      delay(500);
      lcd.clear();
      lcd.print("   Pode passar");
      delay(5000);
      digitalWrite(relay, LOW);
      lcd.clear();
    } else if(saldo == 0 ){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.println("    Sem saldo");
      lcd.setCursor(0,1);
      lcd.print("   suficiente");
      digitalWrite(relay, LOW);
      delay(1500);
      lcd.clear();
    }
    }
      
     if (comparaUID(mfrc522.uid.uidByte, uidcartao2, mfrc522.uid.size)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("     IDOSO");
      delay(1500);
      lcd.clear();
      Serial.println("cartao IDOSO detectado");
      digitalWrite(relay, HIGH);
      lcd.print("   Pode passar");
      Serial.println("Pode passar");
      delay(5000);
      digitalWrite(relay, LOW);
      lcd.clear();
    } else {
      digitalWrite(relay, LOW);
    }
  
    if (comparaUID(mfrc522.uid.uidByte, uidcartao3, mfrc522.uid.size)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    ESTUDANTE");
      delay(1500);
      lcd.clear();
      Serial.println("cartao ESTUDANTE detectado");
      digitalWrite(relay, HIGH);
      lcd.print("   Pode passar");
      Serial.println("Pode passar");
      delay(5000);
      digitalWrite(relay, LOW);
      lcd.clear();
    } else {
      digitalWrite(relay, LOW);
    }

     mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
  }
}

void write(void) {
  delay(50);

   // MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  block = 4;  // Bloco onde o saldo é armazenado

 // mfrc522.PICC_HaltA();
  //mfrc522.PCD_StopCrypto1();

  // Autenticar antes de escrever no cartão
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
   // mfrc522.PICC_HaltA();
   // mfrc522.PCD_StopCrypto1();
    return;
  }

  // Escrever no cartão
  status = mfrc522.MIFARE_Write(block, bufferw, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    //mfrc522.PICC_HaltA();
    //mfrc522.PCD_StopCrypto1();
    return;
  } else {
    Serial.println(F("MIFARE_Write() success"));
  }

  Serial.println("saiu do write");
}

bool verifica_cartao(void) {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  //Serial.println("verifica cartao");
  byte len = 18;
  MFRC522::StatusCode status;

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  Serial.println("tem cartao");
  
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  Serial.println(F("*Card Detected:*"));
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

  byte buffer1[18];
  block = 4;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  Serial.print(F("Saldo: "));
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer1[i] != 32) {
      Serial.write(buffer1[i]);
    }
  }

  char saldoStr[16];
  memcpy(saldoStr, buffer1, 16);
  saldoStr[16] = '\0';
  saldo = strtol(saldoStr, NULL, 10);
  Serial.println();
  delay(1000);

  //mfrc522.PICC_HaltA();
  //mfrc522.PCD_StopCrypto1();

  return true;
}
