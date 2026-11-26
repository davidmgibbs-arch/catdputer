#include "Mistral.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "Roots.h"
bool Mistral::getResponse(const std::vector<ChatMessage>& history, const String& key, const String& model, String& reply) {
 if(WiFi.status()!=WL_CONNECTED){reply="Wi-Fi disconnected. Use :wifi";return false;}
 JsonDocument doc;
 doc["model"]=model; doc["max_tokens"]=512;
 JsonArray messages=doc["messages"].to<JsonArray>();
 JsonObject sys=messages.add<JsonObject>(); sys["role"]="system"; sys["content"]="Answer concisely in plain text for a small screen.";
 for(const auto& m:history){JsonObject o=messages.add<JsonObject>();o["role"]=m.role;o["content"]=m.content;}
 String body; serializeJson(doc,body);
 WiFiClientSecure client; client.setCACert(mistralRoots);
 HTTPClient http; http.setConnectTimeout(15000);http.setTimeout(45000);
 if(!http.begin(client,"https://api.mistral.ai/v1/chat/completions")){reply="HTTPS setup failed";return false;}
 http.addHeader("Content-Type","application/json");http.addHeader("Authorization","Bearer "+key);
 int code=http.POST(body);
 if(code!=200){reply="API/HTTPS error: "+String(code);if(code==401)reply+=" (check key)";if(code==429)reply+=" (rate limit)";http.end();return false;}
 String payload=http.getString();http.end();
 JsonDocument result; if(deserializeJson(result,payload)){reply="Invalid JSON response";return false;}
 if(!result["choices"][0]["message"]["content"].is<const char*>()){reply="No text response";return false;}
 reply=result["choices"][0]["message"]["content"].as<String>();return true;
}
