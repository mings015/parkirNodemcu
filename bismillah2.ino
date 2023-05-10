//include library
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <TimeLib.h>0
#include <Servo.h>
#include <EEPROM.h>
#include <FirebaseArduino.h>




char state;

//#include "FirebaseESP8266.h"
#define WIFI_SSID "LAPTOPKU"
#define WIFI_PASSWORD "123123123"
#define host "kostparkir-default-rtdb.asia-southeast1.firebasedatabase.app"
#define auth "HQs9DDSnjabOhDodMUR1JlTHbk7T1IflFddkxLCc"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800);
//NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);

char Time[ ] = "Nama";
char Waktu[ ] = "Waktu:00:00:00";
char Date[ ] = "Tgl:00-00-2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

#define RST_PIN 3
#define PIN_SS1 D2
#define PIN_SS2 D8
#define JML_READER 2
byte SDA_PIN[] = {PIN_SS1, PIN_SS2};

MFRC522 rfid[JML_READER];
//MFRC522::MIFARE_Key key;


String strID;
//FirebaseData kirim;
String Time_;
String Date_;
String Waktu_;

//set SENSOR IR
int IR = D3;
int IR2 = D0;

Servo servo;
Servo servo2;

void setup() {
  
  servo.attach(D4);
  servo2.attach(D1);
  
  //servo.write(100);
  SPI.begin();
  Serial.begin(9600);
  
  
  
  
  pinMode(IR, INPUT);
  pinMode(IR2, INPUT);
  
  Firebase.begin(host, auth);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  servo.write(220);
  servo2.write(220);

  for (int reader=0; reader < JML_READER; reader++) {
    rfid[reader].PCD_Init(SDA_PIN[reader], RST_PIN);
    Serial.print ("Reader : " + String(reader));
    Serial.print (": ");
    rfid[reader].PCD_DumpVersionToSerial();
    
    }
}


  
void loop() {
  int IRState = digitalRead(IR);
  int IRState2 = digitalRead(IR2);
  
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server
  
  second_ = second(unix_epoch);
  if (last_second != second_) {


    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);

    Time[12] = second_ % 10 + 48;
    Time[11] = second_ / 10 + 48;
    Time[9]  = minute_ % 10 + 48;
    Time[8]  = minute_ / 10 + 48;
    Time[6]  = hour_   % 10 + 48;
    Time[5]  = hour_   / 10 + 48;

    Date[5]  = day_   / 10 + 48;
    Date[6]  = day_   % 10 + 48;
    Date[8]  = month_  / 10 + 48;
    Date[9]  = month_  % 10 + 48;
    Date[13] = (year_   / 10) % 10 + 48;
    Date[14] = year_   % 10 % 10 + 48;
  }
  String Time_(Time);
  String Date_(Date);
  String Waktu_(Waktu);
 
  
  for(int reader=0; reader < JML_READER; reader++){
    if (rfid[reader].PICC_IsNewCardPresent() && rfid[reader].PICC_ReadCardSerial()){
    

    MFRC522::PICC_Type piccType = rfid[reader].PICC_GetType(rfid[reader].uid.sak);


  //id kartu dan yang akan dikirim ke database
    strID = "";
    for (byte i = 0; i < rfid[reader].uid.size; i++) {
    strID +=
      (rfid[reader].uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid[reader].uid.uidByte[i], HEX) + (i != rfid[reader].uid.size - 1 ? ":" : "");
    }
    strID.toUpperCase();
  

  Serial.print("Reader" + String(reader));
  Serial.print(": ");
  rfid[reader].PCD_DumpVersionToSerial();
  Serial.print("STRID :" + strID);
 
  String id_user_izin = Firebase.getString("izin/" + strID + "/id");

  Serial.println(Firebase.getString("penghuni/" + strID));
  String id_user = Firebase.getString("penghuni/" + strID + "/id");
  String nama_user = Firebase.getString("penghuni/" + strID + "/nama");
  String kamar_user = Firebase.getString("penghuni/" + strID + "/kamar");
  
  
  String aktif_user = Firebase.getString("penghuni/" + strID + "/aktif");
  String id_userterparkir = Firebase.getString("parkir/" + strID + "/id");
  Serial.print("Kartu ID Anda : ");
  Serial.println(strID);
  delay(500); 
 
  String waktuu = timeClient.getFormattedTime();
  
//  Serial.println(waktuu);
  //Serial.println(hour_);
  Serial.println(day_);
  Serial.println(month_);
  Serial.println(year_);
  String harimasuk = String(day_) + "-" + String(month_) + "-" + String(year_);
  Serial.println(String(day_) + "-" + String(month_) + "-" + String(year_));
  
  
  int tutupdarijam = 23;
  int hingga = 6;
  int hourku_ = 8;
  
  
  
   
   if (IRState == 0 && reader == 0){
      Serial.println("Silahkan tempelkan kartu");
      delay(500); 

      if (strID == id_user && aktif_user == "no") {
        Serial.println("kartu anda non aktif");
      delay(500); 
      }
      
      else if (strID == id_userterparkir && strID == id_user) {
      Serial.println("anda sudah terparkir");
      delay(500); 

      }
      else if(hourku_ >= tutupdarijam || hourku_ < hingga ) { // harusnya hourku_ jadi hour_ untuk ubah jadi jam sekarang (hourku_ diset manual untuk tes berhasil atau tidak)
        if (strID == id_user_izin && strID != id_userterparkir && aktif_user == "yes") {
            Serial.println ("masuk bang buka servo");
            delay(500);
            Serial.println("nama : ");
            Serial.println (nama_user);
            servo.write(100);
            Serial.println(IRState);
            delay(3000);
            servo.write(200);
            Firebase.setString("parkir/" +id_user + "/id" , id_user);
            Firebase.setString("parkir/" +id_user + "/nama" , nama_user);
            Firebase.setString("parkir/" +id_user + "/kamar" , kamar_user);
            Firebase.setString("parkir/" +id_user + "/jammasuk" , waktuu);
            Firebase.setString("parkir/" +id_user + "/harimasuk" , harimasuk);
      } else {
        Serial.println ("anda belum izin");
      }
        
        Serial.println ("kost tutup tapi bisaji klo memenuhi syarat di fungsi kosttutup");
        
      }       
      else if (strID == id_user) {
        //Serial.println (id_userterparkir);
       delay(500);
       Serial.println("nama : ");
       Serial.println (nama_user);
       servo.write(100);
      // servo2.write(100);
       Serial.println(IRState);
       delay(3000);
       servo.write(200);
      // servo2.write(200);
       Firebase.setString("parkir/" +id_user + "/id" , id_user);
       Firebase.setString("parkir/" +id_user + "/nama" , nama_user);
       Firebase.setString("parkir/" +id_user + "/kamar" , kamar_user);
       Firebase.setString("parkir/" +id_user + "/jammasuk" , waktuu);
       Firebase.setString("parkir/" +id_user + "/harimasuk" , harimasuk);
        
      } 
      
        else {
        delay(500);
        Serial.println("tidak terdaftar");
        }
      
    
    } 
//    else {
//      Serial.println("Objek tidak terdeteksi");
//      delay(500); 
//      Serial.println(IRState);
//      delay(500); 
//      }
    
    
    if(IRState2 == 0 && reader == 1){
      if (strID == id_userterparkir) {
        delay(500);
       Serial.println("nama : ");
       Serial.println (nama_user);
       Serial.println("eksekusi delete");
       servo2.write(100);
      // servo2.write(100);
       Serial.println(IRState2);
       delay(3000);
       servo2.write(200);
       //eksekusi delete
       Firebase.remove("parkir/" + strID);
       Serial.println("berhasil delete");
      } 
      
        
      

        else{
          Serial.println("anda belum parkir");
        }
        
      
    }
    
    
    
     

  }
  }

}





 
