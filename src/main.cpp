#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_SHT31.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>

#define SERVO_PIN PB1
#define BUZZER PD2
#define PIR_PIN PD3
#define SD_CS 10
#define LED_ROSU 4

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {A0, A1, A2, A3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_SHT31 sht30 = Adafruit_SHT31();
RTC_DS3231 rtc;
File logFile;

char currentPin[7] = "1234"; 
char enteredPin[7] = "";
uint8_t pinLength = 0;

unsigned long lastSHTUpdate = 0;
const long shtInterval = 2000;

bool sdCardReady = false;
bool rtcReady = false;
bool shtReady = false;
bool alarmaActivata = false;

// 0 - Normal
// 1 - PIN Vechi
// 2 - PIN Nou
uint8_t modulParola = 0; 

void servo_init(void) {
    DDRB |= (1 << SERVO_PIN);
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 39999;
}

void servo_set_angle(uint8_t angle) {
    if (angle > 180) 
        angle = 180;
    OCR1A = 2000 + (((uint32_t)angle * 2000) / 180);
}

void logToSD(const char* message) {
    if (!sdCardReady) return;

    SD.end();
    delay(50);
    if (!SD.begin(SD_CS)) {
        Serial.println(F("LOG: SD reinit eroare"));
        return;
    }

    logFile = SD.open("log.txt", FILE_WRITE);
    if (!logFile) {
        Serial.println(F("LOG: EROARE"));
        return;
    }

    if (rtcReady) {
        DateTime now = rtc.now();
        char buf[32];
        sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d] ", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
        logFile.print(buf);
    } else {
        logFile.print(F("[NO RTC] "));
    }

    logFile.println(message);
    logFile.close();
    Serial.println(F("LOG: SUCCES"));
}

void declanseazaAlarma() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("INTRUZIUNE!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Alarma activa.    "));
    logToSD("ALARMA DECLANSATA - INTRUS DETECTAT");

    for (int i = 0; i < 30; i++) {
        PORTD |= (1 << BUZZER);
		// led aprins cand suna
        digitalWrite(LED_ROSU, HIGH);
        delay(100);
        
        PORTD &= ~(1 << BUZZER);
		// led stins
        digitalWrite(LED_ROSU, LOW);
        delay(100);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Sistem Securizat"));
    lcd.setCursor(0, 1);
    lcd.print(F("Alarma activata."));
}

void setup() {
    Serial.begin(9600);

    servo_init();
    DDRD |= (1 << BUZZER);
    PORTD &= ~(1 << BUZZER);
    DDRD &= ~(1 << PIR_PIN);
    servo_set_angle(180);

    pinMode(LED_ROSU, OUTPUT);
    digitalWrite(LED_ROSU, LOW);

    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print(F("Pornire sistem.."));
    delay(1000);

    if (sht30.begin(0x44)) shtReady = true;

    if (rtc.begin()) {
        rtcReady = true;
        if (rtc.lostPower())
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println(F("Initializare SD..."));
    if (SD.begin(SD_CS)) {
        sdCardReady = true;
        Serial.println(F("SD: OK!"));
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Sistem Securizat"));
    lcd.setCursor(0, 1);
    lcd.print(sdCardReady ? F("SD: SUCCES!") : F("SD: LIPSA!"));
    delay(1500);
    lcd.clear();
}

void loop() {
    char key = keypad.getKey();
    unsigned long currentMillis = millis();

    // ignor alarma de la senzorul de miscare daca incerc sa tastez codul
    if (alarmaActivata && modulParola == 0 && pinLength == 0 && (PIND & (1 << PIR_PIN))) {
        declanseazaAlarma();
        return;
    }

    // afisez temperatura si umiditatea numai daca ecranul e liber
    if (pinLength == 0 && modulParola == 0 && (currentMillis - lastSHTUpdate >= shtInterval)) {
        lastSHTUpdate = currentMillis;
        if (shtReady) {
            float t = sht30.readTemperature();
            float h = sht30.readHumidity();
            if (!isnan(t) && !isnan(h)) {
                lcd.setCursor(0, 0);
                lcd.print(F("Temp: ")); lcd.print(t, 1); lcd.print((char)223); lcd.print(F("C        "));
                lcd.setCursor(0, 1);
                lcd.print(F("Umid: ")); lcd.print(h, 1); lcd.print(F("%  "));
                
                if (alarmaActivata) {
                    lcd.print(F("[A]"));
                } else {
                    lcd.print(F("[D]"));
                }
            }
        }
    }

    if (key) {
        // sunet taste
        PORTD |= (1 << BUZZER);
        delay(20);
        PORTD &= ~(1 << BUZZER);

        // schimbare parola
        if (key == 'C' && pinLength == 0 && modulParola == 0) {
            modulParola = 1; // verific vechiul pin
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Schimbare parola"));
            lcd.setCursor(0, 1);
            lcd.print(F("PIN vechi: "));
            return; 
        }

        // Activare senzor de miscare
        if (key == 'A' && pinLength == 0 && modulParola == 0) {
            alarmaActivata = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Alarma activata!"));
            lcd.setCursor(0, 1);
            lcd.print(F("Locuinta goala. "));
            logToSD("SENZOR MISCARE ACTIVAT");
            delay(2000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Sistem Securizat"));
            lcd.setCursor(0, 1);
            lcd.print(F("Alarma activata!"));
            return;
        }

        // mesaj cand incepe tastarea
        if (pinLength == 0 && modulParola == 0 && key != '#' && key != '*' && key != 'A' && key != 'C') {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Introduceti PIN:"));
            lcd.setCursor(0, 1);
        }

        // # finalizeaza
        if (key == '#') {
            // verific pin ul vechi
            if (modulParola == 1) {
                if (strcmp(enteredPin, currentPin) == 0) {
                    modulParola = 2; // cer noul pin
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(F("Introduceti"));
                    lcd.setCursor(0, 1);
                    lcd.print(F("noul PIN: "));
                } else {
                    // pin gresit
                    lcd.clear();
                    lcd.print(F("PIN GRESIT!"));
                    delay(2000);
                    modulParola = 0;
                }
            } 
            
            // salvez noul pin
            else if (modulParola == 2) {
                if (pinLength > 0) {
                    strcpy(currentPin, enteredPin);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(F("PIN SCHIMBAT!"));
                    logToSD("PAROLA SISTEMULUI A FOST MODIFICATA");
                    delay(2000);
                } else {
                    lcd.clear();
                    lcd.print(F("Eroare! Anulat."));
                    delay(1500);
                }
                modulParola = 0;
            } 
            
            // starea default
            else {
                if (strcmp(enteredPin, currentPin) == 0) {
                    alarmaActivata = false;
                    lcd.clear();
                    lcd.print(F("ACCES PERMIS"));
                    lcd.setCursor(0, 1);
                    lcd.print(F("Usa deschisa!     "));

                    PORTD |= (1 << BUZZER);
					delay(60);
                    PORTD &= ~(1 << BUZZER);
					delay(40);
                    PORTD |= (1 << BUZZER);
					delay(120);
                    PORTD &= ~(1 << BUZZER);

                    servo_set_angle(0);
                    delay(5000);
                    servo_set_angle(180);
                    delay(500);
                    logToSD("PIN CORECT - ACCES PERMIS");

                } else {
                    lcd.clear();
                    lcd.print(F("PIN GRESIT!"));
                    lcd.setCursor(0, 1);
                    lcd.print(F("Usa blocata!        "));

                    logToSD("PIN GRESIT - ALARMA DECLANSATA");

                    for (int i = 0; i < 15; i++) {
                        PORTD |= (1 << BUZZER);    
                        digitalWrite(LED_ROSU, HIGH);
                        delay(100);
                        
                        PORTD &= ~(1 << BUZZER); 
                        digitalWrite(LED_ROSU, LOW);
                        delay(100);
                    }
                }
            }

            // dupa tasta # curat toate datele introduse
            enteredPin[0] = '\0';
            pinLength = 0;
            lcd.clear();
            
            if (alarmaActivata && modulParola == 0) {
                lcd.setCursor(0, 0);
                lcd.print(F("Sistem Securizat"));
                lcd.setCursor(0, 1);
                lcd.print(F("Alarma activata!"));
            }
        } 
        
        // * reseteaza
        else if (key == '*') {
            enteredPin[0] = '\0';
            pinLength = 0;
            modulParola = 0;
            lcd.clear();
            lcd.print(F("Actiune Anulata"));
            delay(1000);
            lcd.clear();
        } 
        
        // preluarea cifrelor de la tastatura
        else if (key != 'A' && key != 'C') {
            if (pinLength < 6) {
                if (pinLength == 0) {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        if (modulParola == 1) lcd.print(F("Schimbare PIN"));
                        else if (modulParola == 2) lcd.print(F("Noul PIN:"));
                        else lcd.print(F("Introduceti PIN:"));
                        lcd.setCursor(0, 1);
                }
                
                enteredPin[pinLength++] = key;
                enteredPin[pinLength] = '\0';
                lcd.print('*');
            }
        }
    }
}