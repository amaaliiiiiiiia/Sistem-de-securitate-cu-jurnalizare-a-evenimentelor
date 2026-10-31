====== Sistem biometric de securitate cu jurnalizare a evenimentelor ======

===== Introducere =====
Proiectul consta in implementarea unui sistem inteligent de control al accesului intr-o locuinta, fiind bazat pe microcontrolerul ATmega328P. Sistemul utilizeaza o metoda de acces folosind 2 factori, unul biometric printr-un senzor de amprenta si unul printr-un cod numeric introdus de la tastatura, care actioneaza un mecanism de blocare cu servomotor. Din cauza unor limitari hardware severe si a instabilitatii sistemului generate de senzorul biometric, am decis eliminarea acestuia, sistemul final asigurand securitatea doar cu ajutorul codului PIN.

Sistemul se diferentiaza prin capacitatea de a mentine un istoric al interactiunilor pe un card microSD, inregistrand data si ora fiecarei persoane care acceseaza sistemul. In plus, am integrat si un sistem de securitate bazat pe un senzor de miscare PIR, care sa detecteze tentativele de acces neautorizat in interiorul locuintei si sa declanseze o alarma sonora, dar si un modul ce masoara temperatura si umiditatea din locuinta.

Scopul proiectului este realizarea unui sistem de securitate ce nu depinde de o conexiune la internet, care aduce siguranta si care monitorizeaza toate actiunile. Poate fi folosit si pentru securizarea obiectelor de valoare, functionand ca un seif.

===== Descriere generala =====
Microcontrollerul ATmega328P reprezinta modulul central al sistemului, avand rolul de a procesa datele de intrare si de a comanda modulele de iesire. Senzorul de amprenta si tastatura trimit datele de la utilizator catre microcontroller ca acesta sa le verifice. Daca accesul este perimis, servomotorul se deschide, se aude un semnal de succes si se afiseaza un mesaj personalizat pe ecranul LCD. In acelasi timp, modulul RTC ajuta la preluarea datei si orei care apoi se inregistreaza pe cardul SD. Daca accesul nu este permis (amprenta sau codul sunt gresite), se aprinde LED-ul rosu, buzzer-ul emite un sunet de eroare, actiunea se stocheaza in jurnal. Pe langa asta, senzorul de miscare este pregatit sa detecteze miscarea neautorizata din interior, in urma careia va declansa alarma.

Initial, sistemul trebuia sa permita administrarea mai multor utilizatori direct din interfata sa, fara a fi nevoie de un dispozitiv extern. Dupa o autentificare reusita prin cele doua metode, utilizatorul poate accesa meniul de adaugare apasand tasta "A" de pe tastatura matriciala. Se va asocia un ID unic pentru fiecare persoana, conectand astfel amprenta de noul cod PIN. Amprenta va fi scanata si stocata direct in memoria senzorului, iar codul numeric va fi salvat in memoria microcontrollerului, ceea ce asigura persistenta datelor si atunci cand sistemul nu este alimentat cu energie. Prin apasarea tastei "D" de catre administrator, se va putea sterge un utilizator, introducand de la tastatura ID-ul corespunzator acestuia. Toate aceste operatii se vor efectua si cu ajutorul ecranului LCD ce va afisa constant mesaje si instructiuni care sa usureze utilizarea meniului.

In varianta finala si stabila, utilizatorul interactioneaza cu sistemul doar prin intermediul tastaturii matriciale si al ecranului LCD I2C. Daca parola introdusa este corecta, servomotorul deschide usa, se aude un semnal de succes, iar modulul RTC preia data si ora pentru a le inregistra pe cardul SD. Daca accesul nu este permis, se emite un sunet de eroare si actiunea se stocheaza in jurnal. Sistemul ofera si posibilitatea schimbarii parolei direct din interfata (apasand tasta "C"), printr-o masina de stari care valideaza intai vechiul PIN.
Pe langa asta, poate fi activata detectarea miscarii la plecarea din locuinta prin apasarea tastei "A". Senzorul de miscare devine activ si detecteaza miscarea neautorizata din interior, in urma careia va declansa alarma si va bloca sistemul.

{{:pm:prj2026:ionut.otelea:diagrama_acces.png?600|}}
===== Hardware Design =====
**Lista de componente:**
  * ATmega328P Xplained Mini ([[https://ww1.microchip.com/downloads/en/DeviceDoc/50002659A.pdf | Datasheet]])
  * Modul senzor optic de amprente ([[https://handsontec.com/dataspecs/sensor/AS608%20Finger%20print%20Sensor.pdf | Datasheet]]) - eliminat in varianta finala a proiectului
  * Ecran LCD 1602 cu modul I2C ([[https://www.handsontec.com/dataspecs/module/I2C_1602_LCD.pdf | Datasheet]])
  * Senzor PIR HC-SR501 pentru detectia miscarii ([[https://www.handsontec.com/dataspecs/SR501%20Motion%20Sensor.pdf | Datasheet]])
  * Servomotor SG90, 180 grade ([[https://handsontec.com/dataspecs/motor_fan/SG90-Servo.pdf | Datasheet]])
  * Modul DS3231 RTC ([[https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf | Datasheet]])
  * Senzor digital temperatura si umiditate GY-SHT30-D ([[https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf | Datasheet]])
  * Cititor de carduri microSD ([[https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/2189/DFR0229_Web.pdf | Datasheet]])
  * Modul buzzer activ ([[https://www.handsontec.com/dataspecs/module/active%20buzzer%20module.pdf | Datasheet]])
  * Modul LED RGB
  * Tastatura matriciala 4x4
  * Card microSD 16GB
  * Alte componente: Breadboard 830 puncte, fire Dupont, rezistente.

**Schema electrica**

{{:pm:prj2026:ionut.otelea:sch_final_amalia.png?800|}}

**Alocarea pinilor**
^ Componenta ^ Pin ATmega328P ^ Functie ^
| Senzor amprenta (TX) | PD0 | Comunicare seriala (Data In) |
| Senzor amprenta (RX) | PD1 | Comunicare seriala (Data Out) |
| Modul Buzzer | PD2 | Alarme |
| Senzor PIR | PD3 | Detectie miscare |
| LED rosu | PD4 | Status eroare/alarma |
| Servomotor | PB1 | Control acces interior (PWM) |
| Card SD (CS) | PB2 | SPI Chip Select |
| Card SD (MOSI) | PB3 | SPI Master Out Slave In |
| Card SD (MISO) | PB4 | SPI Master In Slave Out |
| Card SD (SCK) | PB5 | SPI Serial Clock |
| LCD / SHT30 / RTC | PC4 | I2C Data (SDA) |
| LCD / SHT30 / RTC | PC5 | I2C Clock (SCL) |
| Tastatura (rand 1) | PD5 | Scanare tastatura rand 1 |
| Tastatura (rand 2) | PD6 | Scanare tastatura rand 2 |
| Tastatura (rand 3) | PD7 | Scanare tastatura rand 3 |
| Tastatura (rand 4) | PB0 | Scanare tastatura rand 4 |
| Tastatura (coloana 1) | PC0 | Scanare tastatura coloana 1 |
| Tastatura (coloana 2) | PC1 | Scanare tastatura coloana 2 |
| Tastatura (coloana 3) | PC2 | Scanare tastatura coloana 3 |
| Tastatura (coloana 4) | PC3 | Scanare tastatura coloana 4 |

===== Software Design =====

Proiectul a fost dezvoltat in mediul **PlatformIO** in Visual Studio Code, oferind un mediu potrivit pentru scrierea codului C++ pentru arhitectura AVR.

==== Mediu de dezvoltare si Librarii ====

Proiectul utilizeaza urmatoarele surse si librarii 3rd-party pentru a integra si controla eficient componentele hardware:

^ Librarie ^ Rol in proiect ^
| LiquidCrystal_I2C | Controlul afisajului LCD 16x2 prin protocol I2C |
| Keypad | Scanarea si debouncing-ul tastaturii matriciale 4x4 |
| Adafruit SHT31 | Citirea datelor de temperatura/umiditate (I2C) |
| RTClib | Gestionarea timpului real (ceas DS3231) |
| SD & SPI | Logarea evenimentelor pe card microSD (SPI) |
| Wire | Implementarea protocolului de comunicare I2C(folosit de LCD, SHT30 si RTC) |

==== Algoritmi si Structuri implementate ====

Pentru optimizarea resurselor limitate ale microcontrollerului ATmega328P (2KB RAM), am implementat urmatoarele strategii:

  * **Optimizarea Memoriei (SRAM):** Am utilizat macro-ul //F()// pentru toate string-urile constante afisate pe LCD sau trimise pe Serial. Aceasta stocheaza mesajele direct in memoria Flash, eliberand memoria RAM, critica pentru buna functionare a librariilor SD si SHT.
  * **Masina de stari:** Am implementat o variabila de stare (//modulParola//) pentru a gestiona fluxul de schimbare a parolei. Sistemul comuta intre stari logice (verificare PIN vechi si setare PIN nou), prevenind astfel necesitatea unor functii de delay.
  * **Manipularea directa a registrilor:**
    * **PWM pentru Servomotor:** In loc sa folosesc o librarie standard care ar fi ocupat resurse suplimentare, am configurat manual registrul //Timer1// (//TCCR1A//, //TCCR1B//) pentru a genera semnale PWM de 50Hz pentru servomotor.
    * **I/O:** Gestionarea buzzerului si a pinilor de intrare PIR este realizata direct prin biti (//PORTD//, //DDRD//), oferind o latenta minima la declansarea alarmei.
  * **Non-Blocking I/O:** Citirea temperaturii si umiditatii se realizeaza folosind functia //millis()//, nu delay(). Aceasta permite sistemului sa scaneze tastatura si sa monitorizeze senzorul PIR in mod continuu, fara a bloca executia.
  * **Stabilizarea Modulului de card SD:** In timpul primelor teste, am observat ca modulul cititor de carduri SD prezenta uneori instabilitate pe magistrala SPI. Pentru a preveni erorile de citire/scriere, am adaugat o rezistenta de pull-up de 47k intre pinul MISO si linia de VCC. Aceasta modificare mentine linia in starea HIGH stabila in momentele de inactivitate, rezolvand definitiv problemele de comunicare.

==== Logica de Securitate si Fluxul Utilizatorului ====

Codul implementeaza un mecanism robust de securitate, bazat pe interactiunea cu tastatura matriciala si validarea in timp real:

  * **Gestionare PIN:** PIN-ul este stocat intr-un vector de caractere (//currentPin//). Cifrele tastate sunt stocate in buffer-ul //enteredPin//, fiind procesate si comparate cu ajutorul functiei //strcmp()// doar la apasarea tastei '#'. In plus, la fiecare cifra tastata, buzzer-ul emite un scurt sunet de 20ms pentru feedback tactil, iar pe ecran apare caracterul '*' in locul fiecarui caracter tastat pentru si mai multa securitate.
  * **Schimbarea Dinamica a Parolei:** Pentru a preveni accesul neautorizat la setarile sistemului, schimbarea PIN-ului se poate face prin apasarea tastei 'C'. Initial se solicita introducerea si validarea parolei curente si doar daca este valida, sistemul permite suprascrierea variabilei //currentPin// folosind functia //strcpy()//.
  * **Modul de Activare a Detectarii Intruziunilor:** Prin apasarea tastei 'A', atunci cand locuinta ramane goala, sistemul intra in modul de securitate maxima, starea aceasta fiind semnalata prin textul '[A]' pentru "ACTIVAT" si '[D]' pentru "DEZACTIVAT", afisat in partea stanga-jos a ecranului. In aceasta stare, pinul hardware asociat senzorului PIR este interogat constant, iar daca se detecteaza miscare, se apeleaza imediat functia //declanseazaAlarma()//.

==== Surse si Functii principale ====

Principalele functii implementate pentru a asigura functionalitatea sistemului sunt:

  * **void servo_init()** / **void servo_set_angle(uint8_t angle)**: Aceste functii controleaza mecanica pentru servomotor, calculand valoarea pentru registrul //OCR1A// necesara pentru a atinge unghiul dorit (0 pentru deschis, 180 pentru inchis).
  * **void logToSD(const char* message)**: Functie de logging care reintegreaza conexiunea SPI cu cardul SD la fiecare apel pentru a evita erorile cauzate de fluctuatiile de tensiune sau de deconectarea accidentala a cardului, garantand integritatea jurnalului. Mesajele sunt prefixate cu un timestamp generat de RTC (ex: [2026-05-26 14:30:00]).
  * **void declanseazaAlarma()**: Functie de securitate care opreste executia normala, afiseaza mejasul de intruziune pe ecranul LCD, logheaza tentativa de acces, activeaza buzzer-ul si aprinde led-ul rosu.
  * **void loop()**: Functioneaza ca un scheduler care in functie de starea sistemului (senzor de miscare activat/dezarmat, meniu parola), decide ce informatii sa actualizeze pe ecran sau ce intrari de la tastatura sa prelucreze.

==== Fragmente de cod relevante ====

**1. Controlul servomotorului**
Pentru a economisi memorie si a avea un control direct asupra semnalului, nu am folosit libraria standard (Servo.h) si am generat semnalul PWM de 50Hz prin configurarea manuala a registrilor de pe Timer1.
**Explicatia matematica:**
Servomotorul SG90 are nevoie de o frecventa fixa de 50Hz, iar un ciclu complet dureaza 20ms. Pentru a incetini ceasul de 16MHz al placii, am folosit un prescaler de 8. Limita superioara a Timer-ului am calculat-o astfel:
  * ''ICR1 = (F_CPU / (Prescaler * F_PWM)) - 1''
  * ''ICR1 = (16000000 / (8 * 50)) - 1 = 39999''
Astfel, Timer1 numara exact 40.000 de tick-uri pentru a masura 20ms.

Pentru a misca bratul, servomotorul se bazeaza pe latimea pulsului: 1ms pentru 0 grade si 2ms pentru 180 grade. Folosind formula de mai sus, putem converti astfel:
  * **0 grade (1ms)** = 40.000 / 20 = **2000 tick-uri**
  * **180 grade (2ms)** = 2000 * 2 = **4000 tick-uri**

Formula din cod gaseste valoarea in tick-uri pentru orice unghi dorit:

<code cpp>
void servo_init(void) {
  DDRB |= (1 << SERVO_PIN);
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
  ICR1 = 39999;
}

void servo_set_angle(uint8_t angle) {
  if (angle > 180) {
    angle = 180;
  }
  // latimea impulsului pe baza unghiului
  OCR1A = 2000 + (((uint32_t)angle * 2000) / 180);
}
</code>

**2. Jurnalizarea robusta pe modulul SD (SPI Auto-Recovery)**
In testele initiale, comunicatia SPI cu cititorul de carduri MicroSD se bloca frecvent. Acest lucru se intampla din cauza liniei MISO care nu se elibera corect atunci cand modulul era inactiv (CS = HIGH). Linia ramanea intr-o stare de floating, capta zgomotul generat de servomotor si bloca magistrala. 

Pe langa solutia hardware de a adauga rezistenta de pull-up de 47k pe linia MISO, am implementat in cod un mecanism software care opreste complet conexiunea SPI (//SD.end()//) si o reinitializeaza fortat inainte de fiecare scriere. Aceasta solutie reseteaza starea comunicatiei si elimina orice eroare, garantand ca fisierele nu se corup si niciun eveniment nu se pierde.

<code cpp>
void logToSD(const char* message) {
  if (!sdCardReady) 
    return;

  // opresc SPI pentru a curata starea
  SD.end();   
  delay(50);
  
  if (!SD.begin(SD_CS)) {
    Serial.println(F("LOG: SD reinit eroare"));
    return;
  }

  logFile = SD.open("log.txt", FILE_WRITE);
  if (logFile) {
    if (rtcReady) {
      DateTime now = rtc.now();
      char buf[32];
      sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d] ", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
      logFile.print(buf);
    }
    logFile.println(message);
    logFile.close();
  }
}
</code>
===== Probleme Intampinate =====

<note warning>
In timpul dezvoltarii, am intampinat dificultati din cauza carora am facut cateva modificari ale designului initial:
</note>

  * **Conflictul Serial si Debugging:** Initial, am conectat senzorul de amprenta la pinii PD0 si PD1 care sunt pinii hardware UART (RX/TX) folositi pentru comunicarea cu PC-ul si pentru debugging. Acest lucru a blocat placa si nu am mai putut incarca codul. Am folosit Microchip Studio pentru debugging, dupa care am eliminat senzorul din proiect pentru a stabiliza placa.
  * **Instabilitatea Alimentarii:** La inceput senzorul de amprenta consuma curent foarte mmult la pornire, ceea ce cauza o cadere de tensiune pe linia de 5V care se observa si in intensitatea luminii ecranului LCD. Am decis sa alimentez senzorul separat cu ajutorul unor baterii, dar acestea nu au rezolvat problema. \\

Din cauza instabilitatii senzorului optic si a limitarilor de curent ale microcontrollerului ATmega328P, am luat decizia sa elimin senzorul de amprenta.

===== Asamblarea fizica a proiectului =====

{{:pm:prj2026:ionut.otelea:securitate1.jpeg?600|}}
{{:pm:prj2026:ionut.otelea:securitate2.jpeg?600|}}
{{:pm:prj2026:ionut.otelea:securitate3.jpeg?600|}}

==== Demonstratie Video si Cod Sursa ====
Aici puteti gasi repository-ul proiectului impreuna cu o demonstratie practica a functionalitatii sistemului:
  * [[https://youtu.be/WTBMvpDCdH8 | Sistem de Securitate cu jurnalizare]]
  * [[https://github.com/amaaliiiiiiiia/Sistem-de-securitate-cu-jurnalizare-a-evenimentelor | Repository GitHub]]
===== Jurnal =====
<note tip>
**29.03.2026** - Am primit componentele pe care le-am comandat \\
**08.05.2026** - Am creat site-ul, am scris primele detalii si am facut schema bloc \\
**11.05.2026** - Am lipit pinii pe placa ATmega328P \\ 
**11.05 - 13.05.2026** - Am conectat 90% din componente si le-am testat daca functioneaza \\
**15.05.2026** - Am facut schema electrica si tabelul cu alocarea pinilor \\
**18.05.2026** - Am lipit fire tata-tata pe firele senzorului de amprenta \\
**18.05 - 20.05.2026** - Am incercat sa conectez senzorul de amprenta, apoi am comandat suport pentru baterii si o noua tastatura deoarece pinii celeilalte s-au defectat \\
**23.05 - 25.05.2026** - Mi-a ajuns comanda si am reincercat sa includ senzorul in proiect, apoi am renuntat la idee si am facut design-ul fizic al proiectului \\
**26.05.2026** - Am finisat proiectul
</note>
