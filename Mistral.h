#pragma once
#include <Arduino.h>
#include <vector>
#include "ChatMessage.h"
class Mistral {
public:
 bool getResponse(const std::vector<ChatMessage>& history, const String& key, const String& model, String& reply);
};
