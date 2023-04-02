#include <Ps3Controller.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "#########";
const char* password = "#########";

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
            <div class="indicators">
                <div class="indicator">
                    <p class="indicator-title">CONNECTED: <strong id="connect">🔴</strong></p>
                </div>
                <div class="indicator">
                    <p class="indicator-title">LX: <strong id="lx">0.00</strong></p>
                </div>
                <div class="indicator">
                    <p class="indicator-title">LY: <strong id="ly">0.00</strong></p>
                </div>
                <div class="indicator">
                    <p class="indicator-title">RX: <strong id="rx">0.00</strong></p>
                </div>
                <div class="indicator">
                    <p class="indicator-title">RX: <strong id="ry">0.00</strong></p>
                </div>
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
    }
    .banner {
        white-space: pre;
    }
    .indicators {
        display: flex;
    }
    .indicator {
        text-align: left;
        width: 20%;
        margin: 1em;
        padding: 1em; 
        border: 2px solid lightblue;
    }
    p {
        margin: 0px;
        text-align: center;
    }
    #terminal {
        margin: 1em;
        padding: 1em; 
        border: 2px solid lightblue;
        height: 400px;
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

    websocket = new WebSocket("ws://192.168.1.34:81/");
    websocket.onmessage = function(event) {

        eventType = event.data.split("|$|")[0]
        eventMessage = event.data.split("|$|")[1]

        if (eventType == "log") {
            printTerminal(eventMessage);
        }
        else if (eventType == "connect") {
            eventMessage = eventMessage.replace("\n","")
            if (eventMessage == "connected") {
                document.getElementById("connect").innerText = "🟢";
            } 
            else {
                document.getElementById("connect").innerText = "🔴";
            }
        } 
        else if (eventType == "lx") {
            document.getElementById("lx").innerText = eventMessage;
        }
        else if (eventType == "ly") {
            document.getElementById("ly").innerText = eventMessage;
        }
        else if (eventType == "rx") {
            document.getElementById("rx").innerText = eventMessage;
        }
        else if (eventType == "ry") {
            document.getElementById("ry").innerText = eventMessage;
        }
        else {
            console.log(event.data)
        }

    };
</script>
)";

void logger(String eventType, String eventMessage) {
    String output = eventType + "|$|" + eventMessage + "\n";
    Serial.print(output);
    websocket.broadcastTXT(output);
}

void onPS3ControllerConnect() {
    logger("connect", "connected");
    logger("log", "PS3 controller connected.");
}

void onPS3ControllerDisconnect() {
    logger("connect", "disconnected");
    logger("log", "PS3 controller disconnected.");
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
  
  if( Ps3.event.button_down.cross )
    logger("log", "✖ Pressed the cross button");
  if( Ps3.event.button_down.square )
    logger("log", "■ Pressed the square button");
  if( Ps3.event.button_down.triangle )
    logger("log", "▲ Pressed the triangle button");
  if( Ps3.event.button_down.circle )
    logger("log", "● Pressed the circle button");
}

void setup(void) {
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
