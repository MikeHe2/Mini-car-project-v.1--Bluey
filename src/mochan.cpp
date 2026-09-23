#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>

/* ================= OLED ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 8
#define OLED_SCL 9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

/* ================= MOTOR PIN ================= */
#define LF   0
#define LB   1
#define RF   2
#define RB   3
#define STBY 10

/* ================= WIFI ================= */
WebServer server(80);
DNSServer dnsServer;

/* ================= STATE ================= */
volatile bool manualActive = false;

/* ================= RANDOM MODE ================= */
enum RandomMode {
  RANDOM_OFF,
  RANDOM_SOFT,
  RANDOM_NORMAL
};

volatile RandomMode randomMode = RANDOM_NORMAL;

/* ================= WIFI MOTOR ================= */
void motorWifi(byte c) {
  digitalWrite(STBY, HIGH);
  switch (c) {
    case 0:
      digitalWrite(LF,LOW); digitalWrite(LB,LOW);
      digitalWrite(RF,LOW); digitalWrite(RB,LOW);
      break;
    case 1:
      digitalWrite(LF,HIGH); digitalWrite(LB,LOW);
      digitalWrite(RF,LOW);  digitalWrite(RB,HIGH);
      break;
    case 2:
      digitalWrite(LF,LOW);  digitalWrite(LB,HIGH);
      digitalWrite(RF,HIGH); digitalWrite(RB,LOW);
      break;
    case 3:
      digitalWrite(LF,LOW);  digitalWrite(LB,HIGH);
      digitalWrite(RF,LOW);  digitalWrite(RB,HIGH);
      break;
    case 4:
      digitalWrite(LF,HIGH); digitalWrite(LB,LOW);
      digitalWrite(RF,HIGH); digitalWrite(RB,LOW);
      break;
  }
}

/* ================= RANDOM MOTOR (GIỮ NGUYÊN) ================= */
void MOTOR(byte c,int t1,int t2,int Time){
  for(int i=0;i<Time;i++){
    switch (c) {
      case 0: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 1: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 2: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
      case 3: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
      case 4: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 5: digitalWrite(LF,LOW); digitalWrite(LB,HIGH);digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 6: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,HIGH);break;
      case 7: digitalWrite(LF,HIGH);digitalWrite(LB,LOW); digitalWrite(RF,LOW); digitalWrite(RB,LOW); break;
      case 8: digitalWrite(LF,LOW); digitalWrite(LB,LOW); digitalWrite(RF,HIGH);digitalWrite(RB,LOW); break;
    }

    delay(t1);

    digitalWrite(LF,LOW); digitalWrite(LB,LOW);
    digitalWrite(RF,LOW); digitalWrite(RB,LOW);

    delay(t2);
  }
}

/* ================= WEB UI ================= */
void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{
  margin:0;height:100vh;
  background:radial-gradient(circle at top,#0f2027,#000);
  color:#00ffe1;
  font-family:Arial;
  display:flex;align-items:center;justify-content:center;
}
.panel{
  width:260px;
  padding:20px;
  border-radius:18px;
  background:rgba(0,255,225,0.05);
  border:1px solid rgba(0,255,225,0.4);
  box-shadow:0 0 25px rgba(0,255,225,0.3);
}
h2{text-align:center;margin:0 0 14px;letter-spacing:2px;}
.grid{
  display:grid;
  grid-template-columns:1fr 1fr 1fr;
  grid-template-rows:60px 60px 60px;
  gap:10px;
}
button{
  border:none;border-radius:12px;
  font-size:16px;font-weight:bold;
  background:linear-gradient(145deg,#0ff,#00b3a4);
}
.stop{background:linear-gradient(145deg,#ff5555,#aa0000);color:#fff;}
.empty{background:none;}

.mode{
  margin-top:12px;
  display:flex;
  gap:6px;
}
.mode button{
  flex:1;
  font-size:13px;
  opacity:0.6;
}
.mode button.active{
  opacity:1;
  background:linear-gradient(145deg,#00ff9c,#00c46a);
  box-shadow:0 0 12px rgba(0,255,180,0.8);
}

.footer{
  margin-top:14px;
  text-align:center;
  font-size:11px;
  opacity:0.6;
  letter-spacing:1px;
}
</style>
</head>
<body>

<div class="panel">
<h2>MOCHAN ROBOT</h2>

<div class="grid">
  <div class="empty"></div>
  <button onclick="fetch('/f')">UP</button>
  <div class="empty"></div>

  <button onclick="fetch('/l')">LEFT</button>
  <button class="stop" onclick="fetch('/s')">STOP</button>
  <button onclick="fetch('/r')">RIGHT</button>

  <div class="empty"></div>
  <button onclick="fetch('/b')">DOWN</button>
  <div class="empty"></div>
</div>

<div class="mode">
  <button id="btn_sleep" onclick="setMode('off')">SLEEP</button>
  <button id="btn_wiggle" onclick="setMode('soft')">WIGGLE</button>
  <button id="btn_curious" class="active" onclick="setMode('normal')">CURIOUS</button>
</div>

<div class="footer">Youtube: Huy Vector</div>
</div>

<script>
function clearActive(){
  document.getElementById('btn_sleep').classList.remove('active');
  document.getElementById('btn_wiggle').classList.remove('active');
  document.getElementById('btn_curious').classList.remove('active');
}

function setMode(mode){
  fetch('/mode_' + mode);
  clearActive();

  if(mode === 'off') document.getElementById('btn_sleep').classList.add('active');
  if(mode === 'soft') document.getElementById('btn_wiggle').classList.add('active');
  if(mode === 'normal') document.getElementById('btn_curious').classList.add('active');
}
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

/* ================= SERVER ================= */
void setupServer() {
  server.on("/", handleRoot);

  server.on("/f", [](){ manualActive=true; motorWifi(1); server.send(200); manualActive=false; });
  server.on("/b", [](){ manualActive=true; motorWifi(2); server.send(200); manualActive=false; });
  server.on("/l", [](){ manualActive=true; motorWifi(3); server.send(200); manualActive=false; });
  server.on("/r", [](){ manualActive=true; motorWifi(4); server.send(200); manualActive=false; });
  server.on("/s", [](){ manualActive=true; motorWifi(0); server.send(200); manualActive=false; });

  server.on("/mode_off",    [](){ randomMode = RANDOM_OFF;    server.send(200); });
  server.on("/mode_soft",   [](){ randomMode = RANDOM_SOFT;   server.send(200); });
  server.on("/mode_normal", [](){ randomMode = RANDOM_NORMAL; server.send(200); });

  server.onNotFound(handleRoot);
  server.begin();
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(STBY,OUTPUT); digitalWrite(STBY,LOW);
  pinMode(LF,OUTPUT); pinMode(LB,OUTPUT);
  pinMode(RF,OUTPUT); pinMode(RB,OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(100000); // 100kHz: mas tolerante con cableado/pull-ups de baja calidad
  Wire.setTimeOut(1000);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); display.display();

  roboEyes.begin(SCREEN_WIDTH,SCREEN_HEIGHT,100);
  roboEyes.setAutoblinker(ON,3,2);
  roboEyes.setIdleMode(ON,2,2);
  roboEyes.setMood(DEFAULT);

  randomSeed(esp_random());

  WiFi.softAP("Bluey");
  dnsServer.start(53,"*",WiFi.softAPIP());
  setupServer();

  digitalWrite(STBY,HIGH);
}

/* ================= LOOP ================= */
void loop() {
  roboEyes.update();
  server.handleClient();
  dnsServer.processNextRequest();

  static unsigned long lastTick = 0;
  if (!manualActive && millis() - lastTick > 40) {
    lastTick = millis();

    if (randomMode == RANDOM_SOFT) {
      if (random(120) == 1) {
        MOTOR(random(9), random(6,18), random(40,90), 1);
      }
    }
    else if (randomMode == RANDOM_NORMAL) {
      if (random(100) == 1) {
        MOTOR(random(9), random(5,50), random(10,100), random(20));
      }
    }
  }
}
