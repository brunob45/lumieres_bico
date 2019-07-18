#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10
MFRC522 module_rfid(SS_PIN, RST_PIN);

int led_rouge = 14;
int led_verte = 15;
int led_mouvement1 = 6;
int led_mouvement2 = 7;
int led_mouvement3 = 8;
int pin_mouvement = 3;
int pin_connu_externe = 4;
int pin_inconnu_externe = 5;

enum ETATS {ETAT_ETEINT, ETAT_INCONNU, ETAT_CONNU, ETAT_MOUVEMENT};
enum RFID {RFID_AUCUN, RFID_INCONNU, RFID_CONNU};

void setup() {
  pinMode(led_rouge, OUTPUT);
  pinMode(led_verte, OUTPUT);
  
  pinMode(led_mouvement1, OUTPUT);
  pinMode(led_mouvement2, OUTPUT);
  pinMode(led_mouvement3, OUTPUT);
  
  pinMode(pin_mouvement, INPUT); 
  pinMode(pin_connu_externe, INPUT); 
  pinMode(pin_inconnu_externe, INPUT); 
  
  SPI.begin();
  module_rfid.PCD_Init();
  Serial.begin(9600);

  delay(3000);
}

// retourne 0 si pas de rfid lue, 1 si rfid est reconnue, 2 si rfid inconnue
int check_rfid()
{
  int statut = RFID_AUCUN;  
  if (module_rfid.PICC_IsNewCardPresent() && module_rfid.PICC_ReadCardSerial())
  {
    String UID = "";
  
    for (byte i = 0; i < module_rfid.uid.size; i++) {
      UID.concat(String(module_rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
      UID.concat(String(module_rfid.uid.uidByte[i], HEX));
    }
  
    UID.toUpperCase();
  
    Serial.println(UID);
    
    if (UID.substring(1) == "32 CE FA 33" 
      ||UID.substring(1) == "D2 2E EA 33"
      ||UID.substring(1) == "E2 FD F3 33"
      ||UID.substring(1) == "A2 D0 F4 33"
      ||UID.substring(1) == "C2 CD FA 33"
      ||UID.substring(1) == "92 06 F4 33"
      ||UID.substring(1) == "32 D1 F4 33"
      ||UID.substring(1) == "62 B6 F1 33"
      ||UID.substring(1) == "32 1B EF 33"
      ||UID.substring(1) == "22 2F EA 33")
    {
      statut = RFID_CONNU;
    } else {
      statut = RFID_INCONNU;
    }
  }
  return statut;
}

bool check_presence() {
  return digitalRead(pin_mouvement);
}

int dernier_changement_etat = 0;
int allumage_led_rouge = 0;
int allumage_yeux = 0;
int etat = ETAT_ETEINT;
int compteur_yeux = 0;

void loop()
{
  int maintenant = millis();
  if(maintenant - dernier_changement_etat < 2000) return;

  int statut = check_rfid();
  
  if(etat == ETAT_ETEINT)
  {
    Serial.println("ETEINT");
    if(statut == RFID_CONNU || digitalRead(pin_connu_externe))
    {
      etat = ETAT_CONNU;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_verte, HIGH);
      digitalWrite(led_rouge, HIGH);
    }
    else if(statut == RFID_INCONNU || digitalRead(pin_inconnu_externe))
    {
      etat = ETAT_INCONNU;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_rouge, HIGH);
      allumage_led_rouge = maintenant;
    }
    else if(check_presence())
    {
      etat = ETAT_MOUVEMENT;
      dernier_changement_etat = maintenant;

      digitalWrite(led_mouvement1, HIGH);
      allumage_yeux = maintenant;
    }
  }
  else if(etat == ETAT_CONNU)
  {
    Serial.println("CONNU");
    if(statut == RFID_CONNU || digitalRead(pin_connu_externe))
    {
      etat = ETAT_ETEINT;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_verte, LOW);
      digitalWrite(led_rouge, LOW);
    }
  }
  else if(etat == ETAT_INCONNU)
  {
    Serial.println("INCONNU");
    if(statut == RFID_CONNU || digitalRead(pin_connu_externe))
    {
      etat = ETAT_CONNU;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_verte, HIGH);
      digitalWrite(led_rouge, HIGH);
    }
    else if(statut == RFID_INCONNU || digitalRead(pin_inconnu_externe))
    {
      allumage_led_rouge = maintenant;
    }
    else if(maintenant - allumage_led_rouge > 4000)
    {
      etat = ETAT_ETEINT;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_rouge, LOW);
    }
  }
  else if(etat == ETAT_MOUVEMENT)
  {
    Serial.println("MOUVEMENT");
    if(statut == RFID_CONNU || digitalRead(pin_connu_externe))
    {
      etat = ETAT_CONNU;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_verte, HIGH);
      digitalWrite(led_rouge, HIGH);
      compteur_yeux = 0;
      digitalWrite(led_mouvement1, LOW);
      digitalWrite(led_mouvement2, LOW);
      digitalWrite(led_mouvement3, LOW);
    }
    else if(statut == RFID_INCONNU || digitalRead(pin_inconnu_externe))
    {
      etat = ETAT_INCONNU;
      dernier_changement_etat = maintenant;
      
      digitalWrite(led_rouge, HIGH);
      allumage_led_rouge = maintenant;
      compteur_yeux = 0;
      digitalWrite(led_mouvement1, LOW);
      digitalWrite(led_mouvement2, LOW);
      digitalWrite(led_mouvement3, LOW);
    }
    else
    {
      if(maintenant - allumage_yeux > 1000)
      {
        allumage_yeux = maintenant;
        compteur_yeux += 1;
        
        if(compteur_yeux % 3 == 0)
        {
          Serial.println("YEUX1");
          digitalWrite(led_mouvement3, LOW);
          digitalWrite(led_mouvement1, HIGH);
        }
        if(compteur_yeux % 3 == 1)
        {
          Serial.println("YEUX2");
          digitalWrite(led_mouvement1, LOW);
          digitalWrite(led_mouvement2, HIGH);
        }
        if(compteur_yeux % 3 == 2)
        {
          Serial.println("YEUX3");
          digitalWrite(led_mouvement2, LOW);
          digitalWrite(led_mouvement3, HIGH);
        }
        if(compteur_yeux >= 12)
        {
          etat = ETAT_ETEINT;
          dernier_changement_etat = maintenant;
      
          compteur_yeux = 0;
          digitalWrite(led_mouvement1, LOW);
          digitalWrite(led_mouvement2, LOW);
          digitalWrite(led_mouvement3, LOW);
        }
      }
    }
  }
}
