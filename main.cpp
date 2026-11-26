#include <M5Cardputer.h>
#include <vector>
#include <WiFi.h>
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
void prompt(int n,const String& message,bool secret=false){stage=n;ui.setSecret(secret);ui.showAssistantMessage(message);}
void setup(){
 auto cfg=M5.config();M5Cardputer.begin(cfg,true);M5Cardputer.Display.setRotation(1);
 static M5Canvas canvas(&M5Cardputer.Display);canvas.createSprite(M5Cardputer.Display.width(),M5Cardputer.Display.height());canvas.setTextSize(1);ui.init(&canvas);
 prefs.begin("mistral",false);ssid=prefs.getString("ssid","");password=prefs.getString("pass","");key=prefs.getString("key","");model=prefs.getString("model","mistral-small-latest");
 if(ssid.isEmpty())prompt(1,"Enter Wi-Fi SSID:");else if(key.isEmpty())prompt(3,"Enter Mistral API key:",true);else connectNetwork();
}
void loop(){
 M5Cardputer.update();ui.handleKeyboardInput();if(!ui.userJustPressedEnter)return;ui.userJustPressedEnter=false;String input=ui.lastEnteredText;ui.lastEnteredText="";
 if(stage==1){ssid=input;prompt(2,"Enter Wi-Fi password (blank for open network):",true);return;}
 if(stage==2){password=input;prefs.putString("ssid",ssid);prefs.putString("pass",password);if(key.isEmpty()){prompt(3,"Enter Mistral API key:",true);return;}stage=0;ui.setSecret(false);connectNetwork();return;}
 if(stage==3){if(input.isEmpty()){prompt(3,"API key cannot be empty:",true);return;}key=input;prefs.putString("key",key);stage=0;ui.setSecret(false);connectNetwork();return;}
 if(stage==4){if(!input.isEmpty()){model=input;prefs.putString("model",model);}stage=0;ui.showAssistantMessage("Model: "+model);return;}
 if(input==":wifi"){prompt(1,"Enter Wi-Fi SSID:");return;}
 if(input==":key"){prompt(3,"Enter new Mistral API key:",true);return;}
 if(input==":model"){prompt(4,"Enter model ID:");return;}
 if(input==":new"){history.clear();ui.showAssistantMessage("New conversation.");return;}
 if(input==":help"){ui.showAssistantMessage(":wifi  Wi-Fi setup\n:key  API key\n:model  Model ID\n:new  Clear chat\n:erase  Erase settings\nFn + ; / .  Scroll");return;}
 if(input==":erase"){prefs.clear();ESP.restart();return;}
 if(input.isEmpty())return;
 while(history.size()>=8)history.erase(history.begin(),history.begin()+2);
 history.push_back({"user",input});ui.showThinking();String reply;
 if(api.getResponse(history,key,model,reply))history.push_back({"assistant",reply});else history.pop_back();ui.showAssistantMessage(reply);
}
