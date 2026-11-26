#include "ChatUI.h"

ChatUI::ChatUI() {
  canvas              = nullptr;
  screenContent       = "";
  userInput           = "";
  scrollOffset        = 0;
  lineHeight          = 10;
  margin              = 3;
  userJustPressedEnter= false;
  lastEnteredText     = "";
}

void ChatUI::init(M5Canvas* c) {
  canvas = c;
}

void ChatUI::clearScreen() {
  if (!canvas) return;
  canvas->fillSprite(TFT_BLACK);
  canvas->pushSprite(0, 0);

  scrollOffset  = 0;
  screenContent = "";
}

// Wrap text explicitly so scrolling matches rendered lines.
static std::vector<String> wrapLines(const String& text, int width) {
  std::vector<String> lines; String line;
  for(unsigned i=0;i<text.length();i++) { char c=text[i]; if(c=='\r')continue; if(c=='\n'){lines.push_back(line);line="";continue;} if((int)line.length()>=width){lines.push_back(line);line="";} line+=c; }
  lines.push_back(line);return lines;
}
int ChatUI::getTotalHeight(const String& text){return wrapLines(text,(canvas->width()-2*margin)/6).size()*lineHeight;}
void ChatUI::drawScreenContent(){if(!canvas)return;canvas->fillSprite(TFT_BLACK);canvas->setTextColor(TFT_WHITE);canvas->setTextWrap(false);int y=-scrollOffset;for(const auto& line:wrapLines(screenContent,(canvas->width()-2*margin)/6)){if(y>=0 && y<canvas->height()){canvas->setCursor(margin,y);canvas->print(line);}y+=lineHeight;}canvas->pushSprite(0,0);}
void ChatUI::typeText(const String& text,bool clearFirst){if(clearFirst)clearScreen();screenContent=text;scrollOffset=0;drawScreenContent();}
void ChatUI::showAssistantMessage(const String& text){typeText(text,true);}
void ChatUI::showThinking(){typeText("Thinking...",true);}
void ChatUI::scrollUp() {
  scrollOffset -= lineHeight;
  if (scrollOffset < 0) scrollOffset = 0;
  drawScreenContent();
}

void ChatUI::scrollDown() {
  int totalH = getTotalHeight(screenContent);
  int screenH = canvas->height();
  int maxOffset = (totalH > screenH) ? (totalH - screenH) : 0;
  scrollOffset += lineHeight;
  if (scrollOffset > maxOffset) scrollOffset = maxOffset;
  drawScreenContent();
}

// The main keyboard handling: scroll, typed input, backspace, Enter
void ChatUI::handleKeyboardInput() {
  if (!canvas) return;
  if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;

  auto ks = M5Cardputer.Keyboard.keysState();

  if(ks.fn){for(auto c:ks.word){if(c==';'){scrollUp();return;}if(c=='.'){scrollDown();return;}}}
  bool changed=false;
  for(auto c:ks.word){if(userInput.length()<2048){userInput+=c;changed=true;}}
  if(ks.del && !userInput.isEmpty()){userInput.remove(userInput.length()-1);changed=true;}
  if(changed){clearScreen();screenContent="> ";if(secret){for(unsigned i=0;i<userInput.length();i++)screenContent+='*';}else screenContent+=userInput;int h=getTotalHeight(screenContent);scrollOffset=max(0,h-canvas->height());drawScreenContent();}
  if(ks.enter){lastEnteredText=userInput;userJustPressedEnter=true;userInput="";}
}
