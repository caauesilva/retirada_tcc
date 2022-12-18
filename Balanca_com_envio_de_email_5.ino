#include <HX711.h> 
#include <LiquidCrystal.h>

#include <Servo.h>

#define SERVO 9

Servo valvula; // Variável Servo

int trava=8;
 
int authorizeID=0;
int authorizeID_check=0;
String accessPerson;

LiquidCrystal lcd(12,11,7,6,5,4);

//configuracao dos pinos para o modulo HX711
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int TEMPO_ESPERA = 1000; //declaracao da variavel de espera
HX711 scale;


unsigned long currentTime=0;
unsigned long pastTime=0;

float authorizedAmount=0;
float weight;


bool authorized_person=false;
bool lcdPrinted=false;
char comando; //declaracao da variavel que ira receber os comandos para alterar o fator de calibracao

void setup ()
{
  valvula.attach(SERVO);
  pinMode(trava,OUTPUT);
  //mensagens do monitor serial
  valvula.write(90);
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  Serial.println("Celula de carga - Calibracao de Peso");
  Serial.println("Posicione um peso conhecido sobre a celula ao comecar as leituras");
  
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-483743);
  scale.tare();               

  //realiza uma media entre leituras com a celula sem carga 
  float media_leitura = scale.get_units(); 
  Serial.print("Media de leituras com Celula sem carga: ");
  Serial.print(media_leitura);
  Serial.println();
  lcd.begin(20,4);
  
  
}

void loop ()
{
currentTime=millis();
/*
if(Serial2.available()>0){
  Serial.write(Serial2.read());
}*/
if(Serial.available()>0){
  Serial2.write(Serial.read());
}


if(Serial2.available()>0){
String esp32Data=Serial2.readStringUntil('\r');
Serial.println(esp32Data);
pastTime=currentTime;
//authorizeID=getFingerprintIDez();
if(esp32Data[0]=='I' && esp32Data[1]=='P'){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(esp32Data.substring(2));
}
if(esp32Data=="AC1"){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Funcionario Caue");
  lcdPrinted=true;
  
  authorizeID=1;
}
else if(esp32Data=="AC2"){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Funcionario Gabriel");  
  lcdPrinted=true;
  authorizeID=3;
}
else if(esp32Data=="NAC"){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Usuario incorreto");  
  lcdPrinted=true;
}




if(authorizeID==1){
  Serial.println("Caue");
   accessPerson="Caue";
   authorizedAmount=0.5;
   authorized_person=true;
}else if(authorizeID==2){
  Serial.println("Luis");
    accessPerson="Luis";
    authorizedAmount=0.5;
    authorized_person=true;    
  }else if(authorizeID==3){
  Serial.println("Gabriel");
    accessPerson="Gabriel";
    authorizedAmount=0.8;
    authorized_person=true;    
  }else if(authorizeID==4){
    Serial.println("Teodoro");
    accessPerson="Teodoro";    
    authorizedAmount=1.2;
    authorized_person=true;
  }
}


if((currentTime-pastTime)>30000 && lcdPrinted==true){lcd.clear();lcdPrinted=false;}

  
if(authorized_person==true){
  Serial2.println(authorizeID);
  delay(5000);
  valvula.write(0);
  do{
    if (scale.is_ready())
  {
    weight=scale.get_units();
    
    lcd.setCursor(7,1);
    lcd.print(weight,2); //retorna a leitura da variavel escala com a unidade quilogramas
    lcd.print(" kg");   
    //Serial.print(weight,2); //retorna a leitura da variavel escala com a unidade quilogramas
    
    
    
}}while(weight<authorizedAmount);
Serial.print(weight);
valvula.write(90);
authorized_person=false;
authorizeID=0;
Serial2.println("ACD");
digitalWrite(trava,HIGH);
delay(3000);
digitalWrite(trava,LOW);
}

  
  
}
