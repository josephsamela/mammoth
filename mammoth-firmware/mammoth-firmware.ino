#include <Ps3Controller.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "#########";
const char* password = "#########";

#define LED_READY  2
int battery = 0;

WebServer server(80);
WebSocketsServer websocket = WebSocketsServer(81);

String site = R"(
<!DOCTYPE html>
<html>
    <head>
        <meta http-equiv="Content-type" content="text/html; charset=utf-8">
        <title>M A M M O T H</title>
    </head>
    <body>
        <div class="container">
            <div class="banner">
███    ███      █████      ███    ███     ███    ███      ██████      ████████     ██   ██
████  ████     ██   ██     ████  ████     ████  ████     ██    ██        ██        ██   ██
██ ████ ██     ███████     ██ ████ ██     ██ ████ ██     ██    ██        ██        ███████
██  ██  ██     ██   ██     ██  ██  ██     ██  ██  ██     ██    ██        ██        ██   ██
██      ██     ██   ██     ██      ██     ██      ██      ██████         ██        ██   ██
            </div>
            <div class="controller">
                <h3>CONTROLLER</h3>
                <table>
                    <tr><td>PS</td><td id="ps">⚪</td></tr>
                    <tr><td>START</td><td id="start">⚪</td></tr>
                    <tr><td>SELECT</td><td id="select">⚪</td></tr>
                        <tr><td> </td><td> </td></tr>
                    <tr><td>LX</td><td id="lx">0</td></tr>
                    <tr><td>LY</td><td id="ly">0</td></tr>
                    <tr><td>RX</td><td id="rx">0</td></tr>
                    <tr><td>RY</td><td id="ry">0</td></tr>
                        <tr><td> </td><td> </td></tr>
                    <tr><td>✖</td><td id="cross">⚪</td></tr>
                    <tr><td>■</td><td id="square">⚪</td></tr>
                    <tr><td>▲</td><td id="triangle">⚪</td></tr>
                    <tr><td>●</td><td id="circle">⚪</td></tr>
                        <tr><td> </td><td> </td></tr>
                    <tr><td>UP</td><td id="up">⚪</td></tr>
                    <tr><td>RIGHT</td><td id="right">⚪</td></tr>
                    <tr><td>DOWN</td><td id="down">⚪</td></tr>
                    <tr><td>LEFT</td><td id="left">⚪</td></tr>
                        <tr><td> </td><td> </td></tr>
                    <tr><td>L1</td><td id="l1">⚪</td></tr>
                    <tr><td>L2</td><td id="l2">0</td></tr>
                    <tr><td>R1</td><td id="r1">⚪</td></tr>
                    <tr><td>R2</td><td id="r2">0</td></tr>
                        <tr><td> </td><td> </td></tr>
                </table>
            </div>
            <div id="terminal"></div>
        </div>
    </body>
</html>

<style>
    body {
        background-color: black;
        color: lightblue;
        text-align: center;
        font-family: monospace;
    }
    .container {
        margin: auto;
        max-width: 800px;
        height: 650px;
        position: absolute;
        left: 50%;
        top: 50%;
        -webkit-transform: translate(-50%, -50%);
        transform: translate(-50%, -50%);
    }
    .banner {
        white-space: pre;
    }
    .controller {
        float: left;
        text-align: left;
        margin: 1em 1em 1em 0px;
        padding: 1em; 
        border: 2px solid lightblue;
        height: 500px;
    }
    table {
        width: 100%;
    }
    h3 {
        margin: 0px 0px 1em 0px;
    }
    td:nth-child(2) {
        text-align: right;
        height: 13px;
    }
    #terminal {
        margin: 1em 0px 1em 1em;
        padding: 1em; 
        border: 2px solid lightblue;
        height: 500px;
        text-align: left;
        font-family: monospace;
        white-space: pre;
        overflow-y: scroll;
    }
</style>

<script>
    function printTerminal(line) {
        t = document.getElementById("terminal")
        time = new Date().toUTCString().split(" ")[4]; // Get current time, hh:mm:ss
        t.insertAdjacentText("beforeend", time + " " + line) // Append text to terminal
        t.scrollTop = t.scrollHeight // Scroll terminal to bottom
    }

    function setButtonState(ref, state) {
        if (state == "true") {
            document.getElementById(ref).innerText = "🟢";
        }
        else if (state == "false") {
            document.getElementById(ref).innerText = "🔴";
        }
        else {
            document.getElementById(ref).innerText = state;
        }
    }

    websocket = new WebSocket("ws://192.168.1.34:81/");

    websocket.onmessage = function(event) {
        eventType = event.data.split("|")[0]
        eventMessage = event.data.split("|")[1]

        if (eventType == "log") {
            printTerminal(eventMessage);
        }
        if (eventType == "connect") { 
            if (eventMessage == "connected") {
                icon  = "🔴"
            }
            if (eventMessage == "disconnected") {
                icon = "⚪"
            }
            ["ps","start","select","cross","square","triangle","circle","up","down","left","right","l1","r1"].forEach(ref => {
                document.getElementById(ref).innerText = icon;
            });
        }
        else {
            button = eventType;
            state = eventMessage;
            setButtonState(button, state);
        }
    };
</script>
)";

void logger(String eventType, String eventMessage) {
    String output = eventType + "|" + eventMessage;
    Serial.print(output);
    websocket.broadcastTXT(output);
}

void onPS3ControllerConnect() {
     String batterystatus;
     battery = Ps3.data.status.battery;
     if( battery == ps3_status_battery_charging )      batterystatus = "CHARGING";
     else if( battery == ps3_status_battery_full )     batterystatus = "FULL";
     else if( battery == ps3_status_battery_high )     batterystatus = "HIGH";
     else if( battery == ps3_status_battery_low)       batterystatus = "LOW";
     else if( battery == ps3_status_battery_dying )    batterystatus = "DYING";
     else if( battery == ps3_status_battery_shutdown ) batterystatus = "SHUTDOWN";
    logger("connect", "connected");
    logger("log", "PS3 controller connected. Battery "+batterystatus+".\n");
}

void onPS3ControllerDisconnect() {
    logger("connect", "disconnected");
    logger("log", "PS3 controller disconnected.\n");
}

void onPS3ControllerInput() {
   // CONTROL LEFT
   if ( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ) {
        logger("lx", String(Ps3.data.analog.stick.lx));
        logger("ly", String(Ps3.data.analog.stick.ly));
    }
   // CONTROL RIGHT
   if ( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ) {
        logger("rx", String(Ps3.data.analog.stick.rx));
        logger("ry", String(Ps3.data.analog.stick.ry));
   }
  
  // CONTROL BUTTONS
    // FACE BUTTONS
  if( Ps3.event.button_down.start )
    logger("start", "true");
  if(Ps3.event.button_up.start)
    logger("start", "false");
  if( Ps3.event.button_down.select )
    logger("select", "true");
  if(Ps3.event.button_up.select)
    logger("select", "false");
  if( Ps3.event.button_down.ps )
    logger("ps", "true");
  if(Ps3.event.button_up.ps)
    logger("ps", "false");

  // FACE BUTTONS
  if( Ps3.event.button_down.cross )
    logger("cross", "true");
  if(Ps3.event.button_up.cross)
    logger("cross", "false");
  if( Ps3.event.button_down.square )
    logger("square", "true");
  if(Ps3.event.button_up.square)
    logger("square", "false");
  if( Ps3.event.button_down.triangle )
    logger("triangle", "true");
  if(Ps3.event.button_up.triangle)
    logger("triangle", "false");
  if( Ps3.event.button_down.circle )
    logger("circle", "true");
  if(Ps3.event.button_up.circle)
    logger("circle", "false");

  // DIRECTION BUTTONS
  if( Ps3.event.button_down.down )
    logger("down", "true");
  if(Ps3.event.button_up.down)
    logger("down", "false");
  if( Ps3.event.button_down.up )
    logger("up", "true");
  if(Ps3.event.button_up.up)
    logger("up", "false");
  if( Ps3.event.button_down.left )
    logger("left", "true");
  if(Ps3.event.button_up.left)
    logger("left", "false");
  if( Ps3.event.button_down.right )
    logger("right", "true");
  if(Ps3.event.button_up.right)
    logger("right", "false");

  // SHOULDER BUTTONS
  if( Ps3.event.button_down.l1 )
    logger("l1", "true");
  if(Ps3.event.button_up.l1)
    logger("l1", "false");
  if( Ps3.event.button_down.r1 )
    logger("r1", "true");
  if(Ps3.event.button_up.r1)
    logger("r1", "false");

  // TRIGGERS
  if( abs(Ps3.event.analog_changed.button.l2) ){
    logger("l2", String(Ps3.data.analog.button.l2));
  }
  if( abs(Ps3.event.analog_changed.button.r2) ){
    logger("r2", String(Ps3.data.analog.button.r2));
  }
}

void setup(void) {
    pinMode(LED_READY,OUTPUT);
    
    // Initialize serial
    Serial.begin(115200);

    // Initialize wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
    Serial.print("\nConnected to " + String(ssid) + "(");
    Serial.print(WiFi.localIP());
    Serial.println(")");

    digitalWrite(LED_READY,HIGH);

    // Initialize PS3 controller
    Ps3.attach(onPS3ControllerInput);
    Ps3.attachOnConnect(onPS3ControllerConnect);
    Ps3.attachOnDisconnect(onPS3ControllerDisconnect);
    Ps3.begin("24:d7:eb:0f:ca:d2");

    // Define route to serve webpage
    server.on("/", []() {
    server.send(200, "text/html", site);
    });

    // Start servers
    websocket.begin();
    server.begin();
}

void loop(void) {
    server.handleClient();
    websocket.loop();
}
