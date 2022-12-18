#include <WiFi.h>
#include <time.h>
#include <ESP_Mail_Client.h>
#include <EEPROM.h>


#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465


// Replace with your network credentials
const char* ssid = "smartscale";
const char* password = "smartscale1";

int led=2;


// Variáveis de envio do email
#define AUTHOR_EMAIL "donotanswer.smartscale@gmail.com"
#define AUTHOR_PASSWORD "prbuljpkzmjqhrcp"
#define RECIPIENT_EMAIL "caues2caue2010@gmail.com"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
String report_hour="19";
String report_day="Saturday";

String hour_day;
String local_hour;  
String local_day;
String messageTXT;
SMTPSession smtp;
int j=0;
float quantidadeMaterial;
float materialGasto;

float materialRecords[10]={14.50,14.00,13.5,13.00,12.50,12.00,11.50,11.00,10.5,10};
float materialRecords2[10];
float variationArray[9];
float variationAVG;
int materialRecordsIndex;
float oldRecord1;
float oldRecord2;


bool amountReport=false;
bool emailSent=false;
String func;
void smtpCallback(SMTP_Status status);
//Fim das Variáveis de envio do email






String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#0}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";
/* Login page */
String loginIndex = 
"<form name=loginForm>"
"<h1>Smart Scale Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(!((form.userid.value=='Caue' && form.pwd.value=='12345') || (form.userid.value=='Gabriel' && form.pwd.value=='12345')))"
"{alert('Usuario ou senha incorretos')}"
"}"
"</script>" + style+
"<!DOCTYPE html><html>"
"<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<link rel=\"icon\" href=\"data:,\">"
"<style>html{font-family: Helvetica;  display: inline-block; margin: 0px auto; text-align: center;}"
".button {background-color: #4CAF50; border: none; color: white; padding: 16px 40px;"
"text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
".button2 {background-color: #555555;}</style></head>"
"<body>"
"<h1>Historico de retiradas</h1>"
"<p>Ate o momento foram utilizados </p>"
"<p>";


String loginIndex2=
"kg essa semana.</p>"
"</body></html>";



String serverIndex1 = 
"<!DOCTYPE html><html>"
"<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<link rel=\"icon\" href=\"data:,\">"
"<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;"
"text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
".button2 {background-color: #555555;}</style></head>"
"<body><h1>Acesso Liberado</h1>"
"<p>Por favor Funcionario";


String serverIndex2 = "Seguir com o processo de retirada</p>"
"</body></html>";

// Set web server port number to 80
WiFiServer server(80);
WiFiClient client;

// Variable to store the HTTP request
String header;


// Current time
unsigned long currentTime = 0;
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
unsigned long pastTime=0;

int interfaceState=1;



void emailSending(float materialAmount,float prediction){
  float material_prediction=materialAmount+prediction;
  ESP_Mail_Session session; 
  SMTP_Message message;  
 smtp.debug(0);
 smtp.callback(smtpCallback); 
 session.server.host_name = SMTP_HOST;
 session.server.port = SMTP_PORT;
 session.login.email = AUTHOR_EMAIL;
 session.login.password = AUTHOR_PASSWORD;
 session.login.user_domain = "";
  
  message.sender.name = "Smart Scale";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Reposicao de Estoque";
  message.addRecipient("Caue", RECIPIENT_EMAIL);
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>A quantidade de material gasta essa semana foi de: "+String(materialAmount)+"kg, para a proxima semana e previsto o uso de: "+String(material_prediction)+"kg</h1><p>-Mensagem Automatica Smart Scale</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))
    return;
 if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Email Error");    
  
}




void setup() {
  EEPROM.begin(80);
  Serial.begin(115200);
  pinMode(led,OUTPUT);
  // Connect to Wi-Fi network with SSID and password
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  delay(2000);
  bool wifiStatusLed=false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
    wifiStatusLed=!wifiStatusLed;
    digitalWrite(led,wifiStatusLed);
  }
  // Print local IP address and start web server
  Serial.print("IP");
  Serial.println(WiFi.localIP());
  digitalWrite(led,HIGH);  
  server.begin();


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  //EEPROM.writeFloat(0,20.00);
  //EEPROM.writeFloat(5,0);
  //EEPROM.commit();
  materialGasto=EEPROM.readFloat(0);  
  //quantidadeMaterial=20;


  
  
}

void loop(){
client = server.available();   // Listen for incoming clients
currentTime = millis();

if(currentTime-pastTime>30000){
  pastTime=currentTime;
  hour_day=printLocalTime();
  local_hour =hour_day.substring(0,hour_day.indexOf('-'));  
  local_day =hour_day.substring(hour_day.indexOf('-')+1,hour_day.length());
  /*
  Serial.print(day);
  Serial.print("-");
  Serial.print(hour);
  Serial.print("-");
  Serial.print(emailSent);
  Serial.print("-");
  Serial.println(quantidadeMaterial);*/
  
if(local_day=="Sunday" && local_hour=="22" && amountReport==false){    
   materialGasto=0;
   EEPROM.writeFloat(0,materialGasto);
   EEPROM.commit();
   amountReport=true;
}
  else if(local_day=="Sunday" && local_hour=="23"){amountReport=false;}

if(local_day==report_day && local_hour==report_hour && emailSent==false){    //colocar o dia e a hora da apresentação (fuso horário +4 se for 7 horas colocar valor 11)
emailSent=true;


for(int i=2;i<12;i++){
if(i==2){oldRecord1=EEPROM.readFloat(5*i);EEPROM.writeFloat(10,materialGasto);EEPROM.commit();materialRecords2[0]=materialGasto;}
oldRecord2=EEPROM.readFloat(5*(i+1));
if(i<11){materialRecords2[i-1]=oldRecord1;}
int writingIndex=5*(i+1);
EEPROM.writeFloat(writingIndex,oldRecord1);EEPROM.commit();
oldRecord1=oldRecord2;
}
for(int i=0;i<9;i++){variationArray[i]=materialRecords2[i]-materialRecords2[i+1];}
for(int i=0;i<9;i++){variationAVG=variationAVG+variationArray[i];}
variationAVG=variationAVG/9;
Serial.println(variationAVG);
emailSending(materialGasto,variationAVG);

Serial.print("Email Enviado");
}else if(local_day==report_day && local_hour=="03"){ //colocar uma hora a mais que o de cima
    emailSent=false;
  }  
}
if(Serial.available()>0){
  digitalWrite(led,LOW);
  func=Serial.readStringUntil('\r');  
  if(func=="1"){materialGasto=materialGasto+0.5;EEPROM.writeFloat(0,materialGasto);EEPROM.commit();Serial.print("MG");Serial.println(materialGasto);}
  else if(func=="2"){materialGasto=materialGasto+0.5;EEPROM.writeFloat(0,materialGasto);EEPROM.commit();Serial.print("MG");Serial.println(materialGasto);}
  else if(func=="3"){materialGasto=materialGasto+0.8;EEPROM.writeFloat(0,materialGasto);EEPROM.commit();Serial.print("MG");Serial.println(materialGasto);}   
  else if(func=="4"){materialGasto=materialGasto+1.2;EEPROM.writeFloat(0,materialGasto);EEPROM.commit();Serial.print("MG");Serial.println(materialGasto);} 
  else if(func=="5"){Serial.print("LH");Serial.print(local_hour);Serial.print("-");Serial.println(local_day);}  
  else if(func[0]=='M'){Serial.println(EEPROM.readFloat(5*(int(func[1])-46)));}
  else if(func=="ACD"){interfaceState=1;}
  else if(func=="resetUsedMass"){EEPROM.writeFloat(0,0.0);EEPROM.commit();materialGasto=0.0;}
  else if(func=="resetUpUsedMass"){EEPROM.writeFloat(0,15.00);EEPROM.commit();materialGasto=15.00;}
  else if(func=="resetMass"){for(int i=2;i<12;i++){EEPROM.writeFloat(5*i,materialRecords[i-2]);EEPROM.commit();}}
  else if(func=="readMass"){for(int i=2;i<12;i++){Serial.print(5*i);Serial.print("-");Serial.println(EEPROM.readFloat(5*i));delay(100);}}  
  else if(func[0]=='H'){report_hour=func.substring(1);Serial.println(report_hour);}
  else if(func[0]=='D'){report_day=func.substring(1);Serial.println(report_day);}
  func="";
  digitalWrite(led,HIGH);   
  }



  if (client){                            // If a new client connects,        
    previousTime = currentTime;
    //Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime-previousTime<=timeoutTime){  // loop while the client's connected
      currentTime=millis();          
      if (client.available()){             // if there's bytes to read from the client,         
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;        
        if (c == '\n') {                         
          if (currentLine.length() == 0){  
         if((header.indexOf("userid=")>=0) && (header.indexOf("&pwd=")>=0)){                                 
          if (header.indexOf("userid=Caue&pwd=12345")>=0){
              Serial.println("AC1");  
              interfaceState=2;                                  
            }else if(header.indexOf("userid=Gabriel&pwd=12345") >= 0) {
              Serial.println("AC2"); 
              interfaceState=3;                         
            }else{
              Serial.println("NAC");
              }
            }
            
            if(interfaceState==1){
              String login_page="";
              /*
              login_page=loginIndex;
              for(int i=2;i<12;i++){login_page+=String(12-i)+":"+String(EEPROM.readFloat(5*i))+"kg ";}
              login_page+=loginIndex2;*/
              login_page=loginIndex+String(materialGasto)+loginIndex2;
              accessPage(login_page);
            }
            
            if(interfaceState==2){accessPage(serverIndex1+" Caue "+serverIndex2);}         
            if(interfaceState==3){accessPage(serverIndex1+" Gabriel "+serverIndex2);}
           
          }else{ // if you got a newline, then clear currentLine
            currentLine="";
          }
        }else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }    
    header = "";   
    
    client.stop();
    
     
    //Serial.println("Client disconnected.");
    //Serial.println("");
  }
  
}


void accessPage(String page){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(page);
    
}


String printLocalTime(){
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    //Serial.println("Failed to obtain time");    
  }
      
  char timeHour[3];
  char timeWeekDay[10];
  strftime(timeHour,3, "%H", &timeinfo);  
  strftime(timeWeekDay,10, "%A", &timeinfo);
  String hour_day = String(timeHour)+"-"+String(timeWeekDay);
  return hour_day;
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  //Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    //Serial.println("----------------");
   // ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
   // ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    //Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      //ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
     // ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      //ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
     // ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
     // ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    //Serial.println("----------------\n");
  }
}
