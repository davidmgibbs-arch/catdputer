#include <M5Cardputer.h>
#include <vector>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_system.h>
#include <Preferences.h>
#include <time.h>
#include "ChatUI.h"
#include "Mistral.h"
static Preferences prefs;
static String ssid, password, key, model;
static ChatUI ui;
static Mistral api;
static std::vector<ChatMessage> history;
static int stage=0;
void connectNetwork(){
 ui.showAssistantMessage("Connecting Wi-Fi...");
 WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(),password.c_str());
 unsigned long start=millis();
 while(WiFi.status()!=WL_CONNECTED && millis()-start<20000){delay(100);}
 if(WiFi.status()!=WL_CONNECTED){ui.showAssistantMessage("Wi-Fi failed. Type :wifi to configure.");return;}
 configTime(0,0,"pool.ntp.org","time.google.com");
 start=millis();while(time(nullptr)<1700000000 && millis()-start<15000)delay(100);
 ui.showAssistantMessage(time(nullptr)<1700000000?"Clock sync failed. Try :wifi again.":"Mistral ready. Type :help for commands.");
}
void configurationPortal(){
 WiFi.disconnect(); WiFi.mode(WIFI_AP);
 char pin[13]; snprintf(pin,sizeof(pin),"%08lx",(unsigned long)esp_random());
 if(!WiFi.softAP("MistralPuter-Setup",pin)){ui.showAssistantMessage("Setup hotspot failed.");return;}
 WebServer server(80); bool saved=false;
 server.on("/",HTTP_GET,[&](){
 server.sendHeader("Cache-Control","no-store");
 server.send(200,"text/html",R"HTML(<!doctype html><meta name="viewport" content="width=device-width,initial-scale=1"><h2>MistralPuter setup</h2><form method="post" action="/save"><p>Wi-Fi name<br><input name="ssid" required></p><p>Wi-Fi password<br><input name="pass" type="password"></p><p>Mistral API key (leave blank to retain existing key)<br><input id="key" name="key" type="password" autocomplete="off"><label><input type="checkbox" onclick="document.getElementById('key').type=this.checked?'text':'password'">Show key</label></p><p>Model<br><input name="model" value="mistral-small-latest" required></p><button>Save and connect</button></form>)HTML");
 });
 server.on("/save",HTTP_POST,[&](){
 if(server.arg("ssid").isEmpty() || (server.arg("key").isEmpty() && key.isEmpty())){server.send(400,"text/plain","Wi-Fi name and API key required. Go back.");return;}
 ssid=server.arg("ssid");password=server.arg("pass");if(!server.arg("key").isEmpty())key=server.arg("key");key.trim();model=server.arg("model");model.trim();
 prefs.putString("ssid",ssid);prefs.putString("pass",password);prefs.putString("key",key);prefs.putString("model",model);
 server.send(200,"text/plain","Saved. Reconnect your phone to your usual Wi-Fi.");saved=true;
 });
 server.begin();ui.setSecret(false);ui.showAssistantMessage(String("Join MistralPuter-Setup\nPassword: ")+pin+"\nOpen http://192.168.4.1\nEnter: skip (if configured)\nTimeout: 5 minutes");
 unsigned long started=millis();while(!saved && millis()-started<300000){server.handleClient();M5Cardputer.update();ui.handleKeyboardInput();if(ui.userJustPressedEnter){ui.userJustPressedEnter=false;ui.lastEnteredText="";if(!ssid.isEmpty() && !key.isEmpty())break;}delay(10);}
 delay(500);server.stop();WiFi.softAPdisconnect(true);stage=0;
 if(saved || (!ssid.isEmpty() && !key.isEmpty()))connectNetwork();else ui.showAssistantMessage("Setup timed out. Type :setup to retry.");
}
void prompt(int n,const String& message,bool secret=false){stage=n;ui.setSecret(secret);ui.showAssistantMessage(message);}
void setup(){
 auto cfg=M5.config();M5Cardputer.begin(cfg,true);M5Cardputer.Display.setRotation(0);
 static M5Canvas canvas(&M5Cardputer.Display);canvas.createSprite(M5Cardputer.Display.width(),M5Cardputer.Display.height());canvas.setTextSize(1);ui.init(&canvas);
 prefs.begin("mistral",false);ssid=prefs.getString("ssid","");password=prefs.getString("pass","");key=prefs.getString("key","");model=prefs.getString("model","mistral-small-latest");
 configurationPortal();
}
void loop(){
 M5Cardputer.update();ui.handleKeyboardInput();if(!ui.userJustPressedEnter)return;ui.userJustPressedEnter=false;String input=ui.lastEnteredText;ui.lastEnteredText="";
 if(input==":setup"){configurationPortal();return;}
 if(stage==1){ssid=input;prompt(2,"Enter Wi-Fi password (blank for open network):",true);return;}
 if(stage==2){password=input;prefs.putString("ssid",ssid);prefs.putString("pass",password);if(key.isEmpty()){prompt(3,"Enter Mistral API key:",true);return;}stage=0;ui.setSecret(false);connectNetwork();return;}
 if(stage==3){if(input.isEmpty()){prompt(3,"API key cannot be empty:",true);return;}key=input;prefs.putString("key",key);stage=0;ui.setSecret(false);connectNetwork();return;}
 if(stage==4){if(!input.isEmpty()){model=input;prefs.putString("model",model);}stage=0;ui.showAssistantMessage("Model: "+model);return;}
 if(input==":wifi"){configurationPortal();return;}
 if(input==":key"){configurationPortal();return;}
 if(input==":model"){prompt(4,"Enter model ID:");return;}
 if(input==":new"){history.clear();ui.showAssistantMessage("New conversation.");return;}
 if(input==":help"){ui.showAssistantMessage(":setup  Phone setup\n:wifi  Wi-Fi setup\n:key  API key\n:model  Model ID\n:new  Clear chat\n:erase  Erase settings\nFn + ; / .  Scroll");return;}
 if(input==":erase"){prefs.clear();ESP.restart();return;}
 if(input.isEmpty())return;
 if(key.isEmpty() || ssid.isEmpty()){configurationPortal();return;}
 while(history.size()>=8)history.erase(history.begin(),history.begin()+2);
 history.push_back({"user",input});ui.showThinking();String reply;
 if(api.getResponse(history,key,model,reply))history.push_back({"assistant",reply});else history.pop_back();ui.showAssistantMessage(reply);
}
