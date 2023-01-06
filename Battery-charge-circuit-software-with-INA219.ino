/*TR*/
/*Sistemde 2'şer şekilde paralel bağlanmış 4 adet pil bulunur.
Pillerin şarj olması için 2 hücre vardır. 
Her pil için ölçümler ayrı ayrı alındığı için sistemde 4 tane akım sensörü kullanılmıştır.
Fakat hücrelerden ortak sonuç alındığı için birbirine paralel olan piller tek pil gibi geri dönüş verirler.
Hücre 1 Pil A ve Pil B , Hücre 2 Pil C ve Pil D şeklindedir.
Yapmış olduğum şarj devresi dışarıdan 220V ile beslenip sistem içindeki converter ile 18V 40A verecek şekilde ayarlanmıştır. 
Her bir pile 18V 10A besleme gider. Pillerin her biri en az 6A'e kadar düşebilir. 6A ölçülen pil sisteme takılmaz, takılsa bile hata verir.
Pil 16.8V ve 0.5A değerlerinde dolu konumundadır ve sistemdeki RGB Ledlerle bulunduğu hücrenin önündeki ledi yeşil yakar. Bir hücrede 2 pil olarak
bulundukları için 2 pilin de 16.8V 0.5A değerlerinde olması gerekir. Sistemde bulunan balancer devreleri eşit şekilde şarj olmalarını sağlarken
Pil A değeri Pil B değerinden farklı bir ölçümdeyse RGB led şarj oluyor olarak mavi yanar. Eğer sistem 6A değerinden küçük ya da eşit şekildeyse 
hata anlamına gelen kırmızı led şeklinde yanar. Sistem içindeki bütün durumlar OLED ekranda görülür. 
*/

/*EN*/
/*There are 4 batteries connected in parallel in 2 ways in the system.
Batteries have 2 cells to charge.
Since the measurements were taken separately for each battery, 4 current sensors were used in the system.
However, since a common result is obtained from the cells, the batteries that are parallel to each other give a return like a single battery.
Cell 1 is Battery A and Battery B, Cell 2 is Battery C and Battery D.
The charging circuit I made is fed with 220V from outside and adjusted to give 18V 40A with the converter inside the system.
Each battery is supplied with 18V 10A. Each of the batteries can go down to at least 6A. The battery measured at 6A is not inserted into the system, it will give an error even if it is inserted.
The battery is full at 16.8V and 0.5A values, and the LED in front of the cell it is in lights green with the RGB LEDs in the system. As 2 batteries in one cell
Since they are present, both batteries must be 16.8V 0.5A. Balancer circuits in the system ensure that they are charged evenly.
If the Battery A value is different from the Battery B value, the RGB LED turns blue as it is charging. If the system is less than or equal to 6A
It lights up as a red LED, which means an error. All the states in the system are seen on the OLED screen.
*/

#include <INA219.h>
#include "OLED.h"
#include <Wire.h>
#include <SPI.h>

//A
#define SHUNT_MAX_V1 0.04
#define BUS_MAX_V1 32
#define MAX_CURRENT1 8
#define SHUNT_R1 0.004

//B
#define SHUNT_MAX_V2 0.04
#define BUS_MAX_V2 32
#define MAX_CURRENT2 8
#define SHUNT_R2 0.004

//C
#define SHUNT_MAX_V3 0.04
#define BUS_MAX_V3 32
#define MAX_CURRENT3 8
#define SHUNT_R3 0.004

//D
#define SHUNT_MAX_V4 0.04
#define BUS_MAX_V4 32
#define MAX_CURRENT4 8
#define SHUNT_R4 0.004

bool flag = false;

//OLED Pin Definition
const int rs = 2,
          rw = 3,
          en = 4,
          d4 = 5,
          d5 = 6,
          d6 = 7,
          d7 = 8; //oled pinleri

Adafruit_CharacterOLED lcd(OLED_V2, rs, rw, en, d4, d5, d6, d7);

//Relay Definition
int Relay1 = 24; //A
int Relay2 = 25; //B
int Relay3 = 26; //C
int Relay4 = 27; //D

//MOSFET Enable
int En = 36; //A
int En1 = 35; //B
int En2 = 39; //C
int En3 = 14; //D

//BirOnuBirBunu Counter
int sayac = 0;

//İNA Definition
INA219 ina219_A((INA219::t_i2caddr) 0x40);
INA219 ina219_B((INA219::t_i2caddr) 0x41);
INA219 ina219_C((INA219::t_i2caddr) 0x44);
INA219 ina219_D((INA219::t_i2caddr) 0x45);

//PCB Led
int led  = 17; // Yeşil led
int led1 = 16;
int led2 = 28;
int led3 = 13; //Teensy default led

int ortalama=9;

// Pil1 RGB LED  c-d led
int Pil1_LED_1 = 29; //B
int Pil1_LED_2 = 30; //R
int Pil1_LED_3 = 32; //G

// Pil2 RGB LED  a-b uyari
int Pil2_LED_1 = 21; //R
int Pil2_LED_2 = 22; //B
int Pil2_LED_3 = 23; //G

//Current Init
float current_mA_A = 0.0;
float current_mA_B = 0.0;
float current_mA_C = 0.0;
float current_mA_D = 0.0;

//Voltage Inıt
float busvoltage_A = 0.0;
float busvoltage_B = 0.0;
float busvoltage_C = 0.0;
float busvoltage_D = 0.0;
float busvoltage_A_reg = 0.0;
float busvoltage_B_reg = 0.0;
float busvoltage_C_reg = 0.0;
float busvoltage_D_reg = 0.0;
float current_mA_A_reg = 0.0;
float current_mA_B_reg = 0.0;
float current_mA_C_reg = 0.0;
float current_mA_D_reg = 0.0;

//Battery 1 Flag Definition
bool pil_1_takili = false;
bool pil_1A_dolu = false;
bool pil_1B_dolu = false;
bool pil_1_hatali = false;
bool onetimeexec = false;

//Battery 2 Flag Definition
bool pil_2_takili = false;
bool pil_2A_dolu = false;
bool pil_2B_dolu = false;
bool pil_2_hatali = false;
bool onetimeexec1 = false;

unsigned stabil_doluluk_sayaci_ab = 0;
unsigned stabil_doluluk_sayaci_cd = 0;

bool pil_birOnuBirBunu = false;

bool flagA, flagB, flagC, flagD = false;
int retA, retB, retC, retD = -1;

/////////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  Serial.begin(9600);
  lcd.begin(20, 4);
  SPI.begin();
  Serial.println("Setup");

  //Açılış Ekranı
  lcd.setCursor(0, 1);
  lcd.print("     Batarya Sarj   ");
  delay(500);
  clrLCDSatir(1);

  retA = ina219_A.begin();
  retB = ina219_B.begin();
  retC = ina219_C.begin();
  retD = ina219_D.begin();

  if (retA == 0) {
    flagA = true;
  } else {
    flagA = false;
  }
  if (retB == 0) {
    flagB = true;
  } else {
    flagB = false;
  }
  if (retC == 0) {
    flagC = true;
  } else {
    flagC = false;
  }
  if (retD == 0) {
    flagD = true;
  } else {
    flagD = false;
  }
  ina219_A.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
  ina219_B.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
  ina219_C.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
  ina219_D.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
  ina219_A.calibrate(SHUNT_R1, SHUNT_MAX_V1, BUS_MAX_V1, MAX_CURRENT1);
  ina219_B.calibrate(SHUNT_R2, SHUNT_MAX_V2, BUS_MAX_V2, MAX_CURRENT2);
  ina219_C.calibrate(SHUNT_R3, SHUNT_MAX_V3, BUS_MAX_V3, MAX_CURRENT3);
  ina219_D.calibrate(SHUNT_R4, SHUNT_MAX_V4, BUS_MAX_V4, MAX_CURRENT4);

  pinMode(led, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  pinMode(Pil1_LED_1, OUTPUT);
  pinMode(Pil1_LED_2, OUTPUT);
  pinMode(Pil1_LED_3, OUTPUT);

  pinMode(Pil2_LED_1, OUTPUT);
  pinMode(Pil2_LED_2, OUTPUT);
  pinMode(Pil2_LED_3, OUTPUT);


  digitalWrite(Pil1_LED_1, LOW);
  digitalWrite(Pil1_LED_2, LOW);
  digitalWrite(Pil1_LED_3, LOW);

  digitalWrite(Pil2_LED_1, LOW);
  digitalWrite(Pil2_LED_2, LOW);
  digitalWrite(Pil2_LED_3, LOW);

  pinMode(Relay1, OUTPUT); //40 - A
  pinMode(Relay2, OUTPUT); //41 - B
  pinMode(Relay3, OUTPUT); //44 - C
  pinMode(Relay4, OUTPUT); //45 - D

  digitalWrite(Relay1, LOW);
  digitalWrite(Relay2, LOW);
  digitalWrite(Relay3, LOW);
  digitalWrite(Relay4, LOW);
  Serial.println("setup roleler icin deaktif konumda");


  pinMode(En , OUTPUT);
  pinMode(En1, OUTPUT);
  pinMode(En2, OUTPUT);
  pinMode(En3, OUTPUT);

  Serial.println("flags of ina");
  Serial.println(flagA);
  Serial.println(flagB);
  Serial.println(flagC);
  Serial.println(flagD);

  Serial.println("returns of ina");
  Serial.println(retA);
  Serial.println(retB);
  Serial.println(retC);
  Serial.println(retD);

  flag = flagA and flagB and flagC and flagD;
  Wire.begin();

  digitalWrite(Pil1_LED_1, HIGH);
  delay(50);
  digitalWrite(Pil1_LED_1, LOW);
  delay(50);
  digitalWrite(Pil1_LED_2, HIGH);
  delay(50);
  digitalWrite(Pil1_LED_2, LOW);
  delay(50);
  digitalWrite(Pil1_LED_3, HIGH);
  delay(50);
  digitalWrite(Pil1_LED_3, LOW);
  delay(50);
  delay(500);
  digitalWrite(Pil2_LED_1, HIGH);
  delay(50);
  digitalWrite(Pil2_LED_1, LOW);
  delay(50);
  digitalWrite(Pil2_LED_2, HIGH);
  delay(50);
  digitalWrite(Pil2_LED_2, LOW);
  delay(50);
  digitalWrite(Pil2_LED_3, HIGH);
  delay(50);
  digitalWrite(Pil2_LED_3, LOW);
  delay(50);
}
/////////////////////////////////////////////////////////////////SETUP/////////////////////////////////////////////////////////////////////////////////////////////////

void clrLCDSatir(int satirNo) {
  lcd.setCursor(0, satirNo);
  lcd.print("                    ");
}

/////////////////////////////////////////////////////////////////LOOP/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  Serial.println("loop");
  digitalWrite(13, HIGH);
  Serial.println("bla bla bla blop");



  if (flag == false) { //İNA N'cONNECT
    Serial.println("hata");
    digitalWrite(17, HIGH);
    digitalWrite(17, LOW);
    while (1);
  }

  retA = ina219_A.begin();
  retB = ina219_B.begin();
  retC = ina219_C.begin();
  retD = ina219_D.begin();

  if (retA == 0) {
    ina219_A.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
    ina219_A.calibrate(SHUNT_R1, SHUNT_MAX_V1, BUS_MAX_V1, MAX_CURRENT1);
    flagA = true;
  }
  else {
    flagA = false;
  }

  if (retB == 0) {
    ina219_B.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
    ina219_B.calibrate(SHUNT_R2, SHUNT_MAX_V2, BUS_MAX_V2, MAX_CURRENT2);
    flagB = true;
  }
  else {
    flagB = false;
  }

  if (retC == 0) {
    ina219_C.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
    ina219_C.calibrate(SHUNT_R3, SHUNT_MAX_V3, BUS_MAX_V3, MAX_CURRENT3);
    flagC = true;
  }
  else {
    flagC = false;
  }

  if (retD == 0) {
    ina219_D.configure(INA219::RANGE_32V, INA219::GAIN_1_40MV, INA219::ADC_64SAMP, INA219::ADC_64SAMP, INA219::CONT_SH_BUS);
    ina219_D.calibrate(SHUNT_R4, SHUNT_MAX_V4, BUS_MAX_V4, MAX_CURRENT4);
    flagD = true;
  }
  else {
    flagD = false;
  }

  Serial.println("returns of ina");
  Serial.println(retA);
  Serial.println(retB);
  Serial.println(retC);
  Serial.println(retD);

  flag = flagA and flagB and flagC and flagD;

  if (flag == true) { //İNA CONNECT / CODE START
    Serial.println("ok");
    ina219_A.recalibrate();
    ina219_A.reconfig();
    ina219_B.recalibrate();
    ina219_B.reconfig();
    ina219_C.recalibrate();
    ina219_C.reconfig();
    ina219_D.recalibrate();
    ina219_D.reconfig();

    digitalWrite(36, HIGH); //A enable
    digitalWrite(35, HIGH); //B enable
    digitalWrite(39, HIGH); //C enable
    digitalWrite(14, HIGH); //D enable
    Serial.println("fet sürücü IC enable edildi");

    for (int i = 0; i < 2; i++) {
      busvoltage_A_reg = busvoltage_A;
      busvoltage_B_reg = busvoltage_B;
      busvoltage_C_reg = busvoltage_C;
      busvoltage_D_reg = busvoltage_D;

      //A-B

      //A                                                 //A voltage measure
      busvoltage_A = ina219_A.busVoltage();
      Serial.print("bus voltage_A:   ");
      Serial.print(busvoltage_A, 4);
      Serial.println(" V");

      //B                                                 //B voltage measure
      busvoltage_B = ina219_B.busVoltage();
      Serial.print("bus voltage_B:   ");
      Serial.print(busvoltage_B, 4);
      Serial.println(" V");

      //C-D

      //C                                                 //C voltage measure
      busvoltage_C = ina219_C.busVoltage();
      Serial.print("bus voltage_C:   ");
      Serial.print(busvoltage_C, 4);
      Serial.println(" V");

      //D                                                 //D voltage measure
      busvoltage_D = ina219_D.busVoltage();
      Serial.print("bus voltage_D:   ");
      Serial.print(busvoltage_D, 4);
      Serial.println(" V");


      if ((abs(busvoltage_A_reg - busvoltage_A) > 0.9) || (abs(busvoltage_B_reg - busvoltage_B) > 0.9) || (abs(busvoltage_C_reg - busvoltage_C) > 0.9) || (abs(busvoltage_D_reg - busvoltage_D) > 0.9)) {
        i = 0;
        continue;
      }
      else {
        break;
      }
    }
    /////////////////////////////////////////////////////////////////////////////////////A-B/////////////////////////////////////////////////////////////////////

    if (busvoltage_A < 2 || busvoltage_B < 2) { ////////////////////////////////////pil takılı değil
      digitalWrite(Relay1, LOW);
      digitalWrite(Relay2, LOW);
      Serial.println("röle1 ve röle 2 deaktif hale getirildi. Pil 1 takili değil");
      pil_1_takili = false;
      pil_1A_dolu = false;
      pil_1B_dolu = false;
      pil_1_hatali = false;
      onetimeexec = false;
      stabil_doluluk_sayaci_ab = 0;

    }

    else if (busvoltage_A < 6 || busvoltage_B < 6) {  ////////////////////////////////////pil hatali
      digitalWrite(Relay1, LOW);
      digitalWrite(Relay2, LOW);
      Serial.println("Pil 1 - 5V altinda - Hatali");
      pil_1_hatali = true;
      pil_1_takili = true;
      stabil_doluluk_sayaci_ab = 0;
    }

    else {                                     ///////////////////////////////////pil takılı
      if (pil_1_hatali == false) {
        if (pil_1A_dolu == false && pil_1B_dolu == false) { //////////////////////////////////sarj olma durumu
          digitalWrite(Relay1, HIGH);
          digitalWrite(Relay2, HIGH);
          delay(500);
          Serial.println("Röle 1 ve röle 2 pil takıldıktan sonra aktif konuma getirildi. !!! ");
          if (onetimeexec == false) {
            busvoltage_A = ina219_A.busVoltage();
            Serial.print("bus voltage_A:   ");
            Serial.print(busvoltage_A, 4);
            Serial.println(" V");

            busvoltage_B = ina219_B.busVoltage();
            Serial.print("bus voltage_B:   ");
            Serial.print(busvoltage_B, 4);
            Serial.println(" V");

          }
        }

        for (int j = 0; j < 2; j++) {
          current_mA_A_reg = current_mA_A;
          current_mA_B_reg = current_mA_B;
          
          current_mA_A =(current_mA_A*ortalama + ina219_A.shuntCurrent())/(ortalama+1);
          Serial.println("current1");
          Serial.print("curr_A:   ");
          Serial.print(current_mA_A, 4);
          Serial.println(" A");

          current_mA_B = (current_mA_B*ortalama + ina219_B.shuntCurrent())/(ortalama+1);
          Serial.println("current2");
          Serial.print("curr_B:   ");
          Serial.print(current_mA_B, 4);
          Serial.println(" A");

          if ((abs(current_mA_A_reg - current_mA_A) > 0.04) || (abs(current_mA_B_reg - current_mA_B) > 0.04)) {
            j = 0;
            continue;
          }
          else {
            break;
          }
        }
          
        pil_1_takili = true;

        if ((current_mA_A < 0.4) && (current_mA_B < 0.4)) {
//DILA          if ((stabil_doluluk_sayaci_ab > 20) || (current_mA_A < 0.05 && current_mA_B < 0.05)) {
            digitalWrite(Relay1, LOW);
            digitalWrite(Relay2, LOW);
            Serial.println("Röle 1 pil_1A şarj tamamlandığı için deaktif konuma getirildi. !!! ");
            Serial.println("Röle 1 pil_1B şarj tamamlandığı için deaktif konuma getirildi. !!! ");
            delay(500);
            busvoltage_A = ina219_A.busVoltage();
            busvoltage_B = ina219_B.busVoltage();
            Serial.print("bus voltage_A:   ");
            Serial.print(busvoltage_A, 4);
            Serial.println(" V");
            Serial.print("bus voltage_B:   ");
            Serial.print(busvoltage_B, 4);
            Serial.println(" V");
            if (busvoltage_A > 2) {
              pil_1A_dolu = true;
            }
            if (busvoltage_B > 2) {
              pil_1B_dolu = true;
            }
//DILA          } else {
//DILA            stabil_doluluk_sayaci_ab = stabil_doluluk_sayaci_ab + 1;
//DILA            Serial.print("pil_1A/1B dolu gibi ama emin değilim; sayıyorum: ");
//DILA            Serial.println(stabil_doluluk_sayaci_ab);
//DILA          }
        }
        else {
          stabil_doluluk_sayaci_ab = 0;
          Serial.println("Pil_1A Sarj oluyor");
          Serial.println("Pil_1B Sarj oluyor");
        }
      }
      else
      {
        Serial.println("Pil_1 hatali olarak isaretlendi !!!");
        if (busvoltage_A > 5 && busvoltage_B > 5) {
          Serial.println("Pil_1 hatali degilmis yanlis anlasilma olmus !!!");
          pil_1_hatali = false;
        }
        stabil_doluluk_sayaci_ab = 0;
      }
    }
    /////////////////////////////////////////////////////////////////////////////////////C-D/////////////////////////////////////////////////////////////////////
    if (busvoltage_C < 2 || busvoltage_D < 2) { //pil takılı değil
      digitalWrite(Relay3, LOW);
      digitalWrite(Relay4, LOW);
      Serial.println("röle3 ve röle4 deaktif hale getirildi. Pil 2 takili değil");
      pil_2_takili = false;
      pil_2A_dolu = false;
      pil_2B_dolu = false;
      pil_2_hatali = false;
      stabil_doluluk_sayaci_cd = 0;
      Serial.println("Pil 2 takili değil");
      current_mA_C = 5000.0;
      current_mA_D = 5000.0;
    }
    else if (busvoltage_C < 6 || busvoltage_D < 6) {
      digitalWrite(Relay3, LOW);
      digitalWrite(Relay4, LOW);
      Serial.println("Pil 2 - 5V altinda - Hatali");
      pil_2_hatali = true;
      pil_2_takili = true;
      stabil_doluluk_sayaci_cd = 0;
    }
    else { //pil takılı
      if (pil_2_hatali == false) {
        if (pil_2A_dolu == false && pil_2B_dolu == false) {
          digitalWrite(Relay3, HIGH);
          digitalWrite(Relay4, HIGH);
          Serial.println("Röle 3 pil takıldıktan sonra aktif konuma getirildi. !!! ");
          delay(500);
          if (onetimeexec1 == false) {
            busvoltage_C = ina219_C.busVoltage();
            Serial.print("bus voltage_C:   ");
            Serial.print(busvoltage_C, 4);
            Serial.println(" V");

            busvoltage_D = ina219_D.busVoltage();
            Serial.print("bus voltage_D:   ");
            Serial.print(busvoltage_D, 4);
            Serial.println(" V");

          }
        }
        for (int k = 0; k < 2; k++) {
          current_mA_C_reg = current_mA_C;
          current_mA_D_reg = current_mA_D;
          current_mA_C =(current_mA_C*ortalama + ina219_C.shuntCurrent())/(ortalama+1);
          Serial.println("current3");
          Serial.print("curr_C:   ");
          Serial.print(current_mA_C, 4);
          Serial.println(" A");

          current_mA_D = (current_mA_D*ortalama + ina219_D.shuntCurrent())/(ortalama+1);
          Serial.println("current4");
          Serial.print("curr_D:   ");
          Serial.print(current_mA_D, 4);
          Serial.println(" A");

          if ((abs(current_mA_C_reg - current_mA_C) > 0.04) || (abs(current_mA_D_reg - current_mA_D) > 0.04)) {
            k = 0;
            continue;
          }
          else {
            break;
          }
        }

        pil_2_takili = true;

        if ((current_mA_C < 0.4) && (current_mA_D < 0.4)) {
//d          if ((stabil_doluluk_sayaci_cd > 20) || (current_mA_C < 0.05 && current_mA_D < 0.05)) {
            digitalWrite(Relay3, LOW);
            digitalWrite(Relay4, LOW);
            Serial.println("Röle 3 pil_2A şarj tamamlandığı için deaktif konuma getirildi. !!! ");
            Serial.println("Röle 4 pil_2B şarj tamamlandığı için deaktif konuma getirildi. !!! ");
            delay(500);
            busvoltage_C = ina219_C.busVoltage();
            busvoltage_D = ina219_D.busVoltage();
            Serial.print("bus voltage_C:   ");
            Serial.print(busvoltage_C, 4);
            Serial.println(" V");
            Serial.print("bus voltage_D:   ");
            Serial.print(busvoltage_D, 4);
            Serial.println(" V");
            if (busvoltage_C > 2) {
              pil_2A_dolu = true;
            }
            if (busvoltage_D > 2) {
              pil_2B_dolu = true;
            }
//d          } else {
//d            stabil_doluluk_sayaci_cd = stabil_doluluk_sayaci_cd + 1;
//d            Serial.print("pil_2A/2B dolu gibi ama emin değilim; sayıyorum: ");
//d            Serial.println(stabil_doluluk_sayaci_cd);
//d          }
        }
        else {
          stabil_doluluk_sayaci_cd = 0;
          Serial.println("Pil_2A Sarj oluyor");
          Serial.println("Pil_2B Sarj oluyor");
        }
      }
      else
      {
        Serial.println("Pil_2 hatali olarak isaretlendi !!!");
        if (busvoltage_C > 5 && busvoltage_D > 5) {
          Serial.println("Pil_2 hatali degilmis yanlis anlasilma olmus !!!");
          pil_2_hatali = false;
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    pil_birOnuBirBunu = !(pil_birOnuBirBunu);
    // test
    float ortABV = (busvoltage_A + busvoltage_B) / 2;
    float ortCDV = (busvoltage_C + busvoltage_D) / 2;

    if (pil_1_takili == false) {                                 //takili degil
      //clrLCDSatir(0);
      //clrLCDSatir(1);
      digitalWrite(Pil1_LED_1, LOW); //Blue
      digitalWrite(Pil1_LED_2, LOW); //Red
      digitalWrite(Pil1_LED_3, LOW); //Green
      lcd.setCursor(0, 0);
      lcd.print("Pil 1 Takili Degil    ");

    }
    else {                                                         //takili

      if (pil_1_hatali == true) {                                //hatali
        //clrLCDSatir(0);
        //clrLCDSatir(1);
        lcd.setCursor(0, 0);
        lcd.print("Pil 1 hatali        ");
        digitalWrite(Pil1_LED_2, HIGH); //Red
        digitalWrite(Pil1_LED_1, LOW); //Blue
        digitalWrite(Pil1_LED_3, LOW); //Green
        Serial.println("Pil 1 hatali        ");

      }
      else {                                                       //hatali degil
        if (pil_1A_dolu == true && pil_1B_dolu == true) {
          //clrLCDSatir(0);
          //clrLCDSatir(1);
          lcd.setCursor(0, 0);
          lcd.print("pil 1 dolu          ");
          digitalWrite(Pil1_LED_3, HIGH); //green
          digitalWrite(Pil1_LED_1, LOW); //blue
          digitalWrite(Pil1_LED_2, LOW); //red
          Serial.println("pil 1 dolu          ");
        }
        else {
          //clrLCDSatir(0);
          //clrLCDSatir(1);
          digitalWrite(Pil1_LED_1, HIGH); //blue
          digitalWrite(Pil1_LED_3, LOW); //green
          digitalWrite(Pil1_LED_2, LOW); //red


          if (pil_birOnuBirBunu == false) {
            lcd.setCursor(0, 0);
            lcd.print("Pil 1 Sarj Oluyor    ");
          }
          /*else {
               //clrLCDSatir(0);
               int ABX = map((int) ortABV, 14, 17, 0, 100);
               lcd.setCursor(0, 0);
               lcd.print("Pil_1: ");
               lcd.setCursor(7, 0);
               lcd.print("%");
               lcd.print(ortABV);
               lcd.setCursor(12, 0);
               lcd.print("V");
               Serial.println("////////////////////////////////////////////////////////");
               Serial.print("Pil_1 :  ");
               Serial.println(ortABV);
               Serial.println("////////////////////////////////////////////////////////");
            }*/
        }
      }
    }
    if (pil_2_takili == false) {
      //clrLCDSatir(2);
      //clrLCDSatir(3);
      digitalWrite(Pil2_LED_2, LOW); //Blue
      digitalWrite(Pil2_LED_1, LOW); //Red
      digitalWrite(Pil2_LED_3, LOW); //Green
      lcd.setCursor(0, 3);
      lcd.print("Pil 2 Takili Degil  ");
    }
    else {
      if (pil_2_hatali == true) {
        //clrLCDSatir(2);
        //clrLCDSatir(3);
        lcd.setCursor(0, 3);
        lcd.print("Pil 2 Hatali        ");
        digitalWrite(Pil2_LED_1, HIGH); //Red
        digitalWrite(Pil2_LED_2, LOW); //Blue
        digitalWrite(Pil2_LED_3, LOW); //Green
        Serial.println("Pil 2 Hatali        ");
      }
      else {
        if (pil_2A_dolu == true && pil_2B_dolu == true) {
          //clrLCDSatir(2);
          //clrLCDSatir(3);
          lcd.setCursor(0, 3);
          lcd.print("pil 2 dolu          ");
          digitalWrite(Pil2_LED_3, HIGH); //green
          digitalWrite(Pil2_LED_1, LOW);  //red
          digitalWrite(Pil2_LED_2, LOW);  //blue
        }
        else {
          //clrLCDSatir(2);
          //clrLCDSatir(3);
          digitalWrite(Pil2_LED_2, HIGH); //blue
          digitalWrite(Pil2_LED_1, LOW); //red
          digitalWrite(Pil2_LED_3, LOW); //green

          if (pil_birOnuBirBunu == false) {
            lcd.setCursor(0, 3);
            lcd.print("Pil 2 Sarj Oluyor   ");
          }
          /*else {
               //clrLCDSatir(3);
               int CDX = map((int) ortCDV, 14, 17, 0, 100);
               lcd.setCursor(0, 3);
               lcd.print("Pil_2: ");
               lcd.setCursor(7, 3);
               lcd.print("%");
               lcd.print(ortCDV);
               lcd.setCursor(12, 3);
               lcd.print("V");
               Serial.println("////////////////////////////////////////////////////////");
               Serial.print("Pil_2 :  ");
               Serial.println(ortCDV);
               Serial.println("////////////////////////////////////////////////////////");
            }*/
          else {
            Serial.println("////////////////////////////////////////////////////////");
            Serial.print("Pil_2 :  ");
            Serial.println(ortCDV);
            Serial.println("////////////////////////////////////////////////////////");
          }
        }
      }
    }


  }
  delay(500);
}

/////////////////////////////////////////////////////////////////LOOP/////////////////////////////////////////////////////////////////////////////////////////////////
