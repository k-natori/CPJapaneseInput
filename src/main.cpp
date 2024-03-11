#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5GFX.h>
#include <WiFi.h>
#include "JPutil.h"

#include "TextAction.h"
#include "SaveAction.h"
#include "QRAction.h"
#include "BlueSkyAction.h"
#include "EMailAction.h"

#define defaultKanaDicFileName "/kanadic.txt"
#define defaultSKKDicFileName "/SKK-JISYO.M"

typedef enum
{
  TextScreen,
  ActionListScreen,
  ActionPrepareScreen,
  ActionDoneScreen,
} ScreenType;

// Screen variables
M5GFX lcd;

#define screenWidth 240
#define screenHeight 135
#define inputLineHeight 27
int textColor = CYAN;
int backgroundColor = BLACK;

ScreenType currentScreen = TextScreen;
boolean kanaActive = false;
boolean caretActive = false;

// Text handling
String screenBuffer = "";
String inputLineBuffer = "";
String romanBuffer = "";
String kanaBuffer = "";

String lastRoman = "";
String lastKana = "";
std::vector<String> kanjiVector;
int16_t kanjiIndex = -1;

// Text actions (see TextAction.h for details)
SaveAction saveAction;
QRAction qrAction;
BlueSkyAction blueSkyAction;
EMailAction emailAction;
std::vector<TextAction *> actionVector = {&saveAction, &qrAction, &blueSkyAction, &emailAction};
int actionIndex = -1;

// Function prototypes
void showActionScreen();
boolean prepareAction(int index);
void doAction(int index);
void showTextScreen();
void updateScreen(String newString);
void addScreenString(String newString);
void updateInputLine(String newString);
void blinkCaret();

void pushFnNumber(int fnNumber);
void pushEnter(boolean fnPushed);
void pushSpace(boolean fnPushed);
void pushChar(char c);
void pushBackSpace(boolean fnPushed);
void pushEscape(boolean fnPushed);
void pushLeftArrow();
void pushRightArrow();
void pushUpArrow();
void pushDownArrow();

void resetKanjiConversion();

// Setup called when booting
void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  SPI.begin(40, 39, 14, 12);
  SD.begin(12);
  lcd = M5Cardputer.Display;

  // Load settings from "settings.txt" in SD card
  String wifiIDString = "wifiID";
  String wifiPWString = "wifiPW";
  String kanaDicFileName = "/kanadic.txt";
  String SKKDicFileName = "/SKK-JISYO.M";
  String blueSkyPEMFileName = "";
  File settingFile = SD.open("/settings.txt");
  if (settingFile)
  {
    while (settingFile.available() > 0)
    {
      String line = settingFile.readStringUntil('\n');
      if (line.startsWith("//"))
        continue;
      int separatorLocation = line.indexOf(":");
      if (separatorLocation > -1)
      {
        String key = line.substring(0, separatorLocation);
        String content = line.substring(separatorLocation + 1);

        // WiFi SSID and paassword
        if (key == "SSID")
          wifiIDString = content;
        else if (key == "PASS")
          wifiPWString = content;

        // Kana conversion dictionaries
        else if (key == "KanaDicFileName")
          kanaDicFileName = content;
        else if (key == "SKKDicFileName")
          SKKDicFileName = content;

        // BlueSky access
        else if (key == "BlueSkyPEM")
          BlueSkyAction::setPEMFileName(content);
        else if (key == "BlueSkyName")
          BlueSkyAction::setAccountName(content);
        else if (key == "BlueSkyPass")
          BlueSkyAction::setAppPassword(content);

        // E-mail
        else if (key == "GMailAccount")
          EMailAction::setAccountName(content);
        else if (key == "GMailPassword")
          EMailAction::setPassword(content);
        else if (key == "EMailReceiver")
          EMailAction::setReceiver(content);
        else if (key == "EMailSubjectLine")
          EMailAction::setMakeFirstLineSubject(content.length() > 0);
      }
    }
    settingFile.close();

    // Start Wifi connection
    WiFi.begin(wifiIDString.c_str(), wifiPWString.c_str());
  }

  // Screen
  textColor = CYAN;
  backgroundColor = BLACK;

  lcd.setRotation(1);
  lcd.fillScreen(textColor);
  lcd.setTextColor(backgroundColor);
  lcd.setTextDatum(top_left);
  lcd.setFont(&fonts::lgfxJapanGothic_24);
  lcd.setTextSize(1);
  lcd.setCursor(0, 2);

  if (loadKanaDicFile(kanaDicFileName))
  {
    lcd.printf("%s loaded\n", kanaDicFileName);
  }
  else
  {
    lcd.printf("%s not found in SD\n", kanaDicFileName);
  }

  if (loadKanjiDicFile(SKKDicFileName))
  {
    lcd.printf("%s loaded\n", SKKDicFileName);
  }
  else
  {
    lcd.printf("%s not found in SD\n", SKKDicFileName);
  }

  lcd.printf("min free(): %6d\n", esp_get_minimum_free_heap_size());
  lcd.print("Push any key");

  currentScreen = ActionDoneScreen;
  // loadLastFile();
  // showTextScreen();
}

// loop called repeatedly while power on
void loop()
{
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange())
  {

    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      if (currentScreen == ActionDoneScreen)
      {
        showTextScreen();
        return;
      }

      if (status.enter)
      {
        pushEnter(status.fn);
      }
      else if (status.space)
      {
        pushSpace(status.fn);
      }
      else if (status.del)
      {
        pushBackSpace(status.fn);
      }
      else
      {

        if (status.word.size() == 0)
          return;
        char c = status.word.back();
        Serial.printf("%x\n", c);
        if (c >= 0x21 && c <= 0x7E)
        {
          if (c == '`' && (status.fn || status.alt))
          {
            pushEscape(status.fn);
          }
          /*
          else if (status.fn && c >= 49 && c <= 57)
          {
            pushFnNumber(c - 48);
          }
          */
          else if (currentScreen == ActionListScreen)
          {
            switch (c)
            {
            case ',':
            {
              pushLeftArrow();
              return;
            }
            case '/':
            {
              pushRightArrow();
              return;
            }
            case ';':
            {
              pushUpArrow();
              return;
            }
            case '.':
            {
              pushDownArrow();
              return;
            }

            default:
            {
              showTextScreen();
              return;
            }
            }
          }
          else
          {
            pushChar(c);
          }
        }
        else if (c == 0x1B)
        {
          pushEscape(status.fn);
        }
      }
    }
  }
  if (currentScreen == TextScreen)
    blinkCaret();
  delay(10);
}

// Show actions screen
void showActionScreen()
{
  currentScreen = ActionListScreen;
  lcd.fillRect(0, 0, screenWidth, screenHeight, backgroundColor);
  for (int i = 0; i < actionVector.size(); i++)
  {
    int tempTextColor = (i == actionIndex) ? backgroundColor : textColor;
    int tempBackgroundColor = (i == actionIndex) ? textColor : backgroundColor;

    lcd.fillRect(0, inputLineHeight * i, screenWidth, inputLineHeight, tempBackgroundColor);
    lcd.setTextColor(tempTextColor);
    lcd.setCursor(0, inputLineHeight * i + 2);
    // lcd.printf("F%d:", i + 1);
    lcd.print(actionVector[i]->title());
    lcd.drawFastHLine(0, inputLineHeight * (i + 1), screenWidth, tempTextColor);
  }
}

// Prepare selected action
boolean prepareAction(int index)
{
  if (index < 0 || index >= actionVector.size())
    return false;
  currentScreen = ActionPrepareScreen;
  TextAction *action = actionVector[actionIndex];
  String message = action->message();
  String OKTitle = action->OKTitle();
  String cancelTitle = action->cancelTitle();
  boolean result = true;
  if (message.length() > 0)
  {
    lcd.fillRect(0, 0, screenWidth, screenHeight, textColor);
    lcd.setTextColor(backgroundColor);
    lcd.setCursor(0, 2);
    lcd.print(message);

    if (OKTitle.length() > 0)
    {
      lcd.drawFastHLine(0, screenHeight - (inputLineHeight + 1), screenWidth, backgroundColor);
      lcd.setCursor(screenWidth / 2 + 5, screenHeight - inputLineHeight + 2);
      lcd.print(OKTitle);

      if (cancelTitle.length() > 0)
      {
        lcd.drawFastVLine(screenWidth / 2, screenHeight - (inputLineHeight + 1), inputLineHeight, backgroundColor);
        lcd.setCursor(5, screenHeight - inputLineHeight + 2);
        lcd.print(cancelTitle);
      }
      result = false;
    }
  }
  if (action->prepareAction(lcd) == true)
  {
    return result;
  }
  showTextScreen();
  return false;
}

// Perform selected action
void doAction(int index)
{
  if (index < 0 || index >= actionVector.size())
    return;
  currentScreen = ActionDoneScreen;
  TextAction *action = actionVector[actionIndex];
  if (action->doAction(lcd, screenBuffer))
  {
    showTextScreen();
  }
}

// Switch to text input screen
void showTextScreen()
{
  currentScreen = TextScreen;
  lcd.setTextColor(textColor);
  updateScreen(screenBuffer);
  if (kanaActive)
  {
    lcd.drawFastHLine(0, screenHeight - (inputLineHeight + 1), screenWidth, textColor);
    updateInputLine(inputLineBuffer);
  }
}

// update main text screen area
void updateScreen(String newString)
{
  screenBuffer = newString;
  if (kanaActive)
    lcd.fillRect(0, 0, screenWidth, screenHeight - (inputLineHeight + 1), backgroundColor);
  else
    lcd.fillRect(0, 0, screenWidth, screenHeight, backgroundColor);
  lcd.setCursor(2, 2);
  lcd.print(screenBuffer);
}

// add text to main text area
void addScreenString(String newString)
{
  screenBuffer += newString;
  if (caretActive)
  {
    int x = lcd.getCursorX();
    int y = lcd.getCursorY();
    lcd.fillRect(x, y, 2, 24, backgroundColor);
  }
  lcd.setCursor(2, 2);
  lcd.print(screenBuffer);
}

// update input line screen area
void updateInputLine(String newString)
{
  inputLineBuffer = newString;
  lcd.fillRect(0, screenHeight - (inputLineHeight), screenWidth, inputLineHeight, backgroundColor);
  lcd.setCursor(2, screenHeight - inputLineHeight + 2);
  lcd.print(inputLineBuffer);
}

// Blink caret
void blinkCaret()
{
  static unsigned long prevMillis;
  unsigned long currentMillis = millis();
  if (currentMillis > prevMillis + 500)
  {
    prevMillis = currentMillis;
    caretActive = !caretActive;
    int x = lcd.getCursorX();
    int y = lcd.getCursorY();
    int color = caretActive ? textColor : backgroundColor;
    lcd.fillRect(x, y, 2, 24, color);
  }
}

// Function keys
void pushFnNumber(int fnNumber)
{
  fnNumber--;
  if (fnNumber >= 0 && fnNumber < actionVector.size())
  {
    if (prepareAction(fnNumber))
    {
      doAction(fnNumber);
    }
  }
}

// push enter key. change line or transfer input line to screen
void pushEnter(boolean fnPushed)
{
  switch (currentScreen)
  {
  case TextScreen:
  {
    if (fnPushed)
    { // Fn + Enter: Show action list screen
      showActionScreen();
      return;
    }
    else
    {
      if (kanaActive)
      { // Kana
        if (inputLineBuffer.length() == 0)
        {
          inputLineBuffer = "\n";
        }
        String newString = screenBuffer + inputLineBuffer;
        updateScreen(newString);
        updateInputLine("");
        romanBuffer = "";
        kanaBuffer = "";
        lastRoman = "";
        lastKana = "";
        resetKanjiConversion();
      }
      else
      { // Alphabet
        addScreenString("\n");
      }
      return;
    }
  }
  case ActionListScreen:
  {
    if (fnPushed)
    { // Fn + Enter: Back to text input screen
      showTextScreen();
      return;
    }
    else
    {
      if (actionIndex >= 0 && actionIndex < actionVector.size())
      {
        pushFnNumber(actionIndex + 1);
      }
      return;
    }
  }
  case ActionPrepareScreen:
  {
    if (actionIndex > -1)
    {
      // Waiting prepare message
      if (actionIndex < actionVector.size())
      {
        boolean result = actionVector[actionIndex]->doAction(lcd, screenBuffer);
        if (result)
        {
          showTextScreen();
        }
      }
      actionIndex = -1;
      return;
    }
  }
  default:
    break;
  }
}

// push space key. insert space or transform kana to kanji
void pushSpace(boolean fnPushed)
{
  if (fnPushed)
  { // Fn + Space: Switch to kana input mode
    kanaActive = !kanaActive;
    showTextScreen();
  }
  else
  {
    if (kanaActive)
    { // Kana
      if (kanaBuffer.length() > 0)
      {
        size_t count = kanjiVector.size();
        if (kanjiIndex == -1)
        {
          if (count == 0)
          {
            // Search kanji entries in SKK dictionary
            kanjiVector = kanjiVectorFromKana(kanaBuffer, lastRoman, lastKana);

            // Add katakana entry
            String katakana = katakanaFromHiragana(kanaBuffer);
            kanjiVector.push_back(katakana);
            count = kanjiVector.size();
          }
        }

        kanjiIndex++;
        if (count > 0 && count > kanjiIndex)
        {
          String kanji = kanjiVector[kanjiIndex];
          if (kanji.length() > 0)
          {
            Serial.println(kanaBuffer + "->" + kanji);
            updateInputLine(kanji);
          }
        }
        else
        {
          updateInputLine(kanaBuffer);
          kanjiIndex = -1;
        }
      }
    }
    else
    { // Alphabet
      addScreenString(" ");
    }
  }
}

// Character key pushed
void pushChar(char c)
{
  if (kanaActive)
  { // Kana

    if (kanjiIndex > -1)
    {
      pushEnter(false);
    }

    // Convert roman to kana
    romanBuffer += c;
    String kana = kanaFromRoman(romanBuffer);
    if (kana.length() > 0)
    {
      // Kana found
      Serial.println(romanBuffer + "->" + kana);
      lastRoman = romanBuffer;
      lastKana = kana;

      romanBuffer = "";
      kanaBuffer += kana;
    }

    updateInputLine(kanaBuffer + romanBuffer);
  }
  else
  { // Alphabet
    addScreenString(String(c));
  }
}

// Backspace key pushed
void pushBackSpace(boolean fnPushed)
{

  if (kanaActive)
  { // Kana
    resetKanjiConversion();
    if (romanBuffer.length() > 0)
    {
      romanBuffer = romanBuffer.substring(0, romanBuffer.length() - 1);
      updateInputLine(kanaBuffer + romanBuffer);
    }
    else if (kanaBuffer.length() > 0)
    {
      kanaBuffer = removeLastMultiByteChar(kanaBuffer);
      updateInputLine(kanaBuffer);
    }
    else if (screenBuffer.length() > 0)
    {
      screenBuffer = removeLastMultiByteChar(screenBuffer);
      updateScreen(screenBuffer);
    }
  }
  else
  { // Alphabet
    screenBuffer = removeLastMultiByteChar(screenBuffer);
    updateScreen(screenBuffer);
  }
}

void pushEscape(boolean fnPushed)
{
  if (actionIndex > -1)
  {
    showTextScreen();
    actionIndex = -1;
    return;
  }

  if (!fnPushed)
  { // Fn + ESC: Switch to kana mode
    kanaActive = !kanaActive;
    showTextScreen();
  }
  else
  {
    if (currentScreen == TextScreen)
    {
      showActionScreen();
    }
    else if (currentScreen == ActionListScreen)
    {
      showTextScreen();
    }
  }
}

// Virtual arrow keys to select action
void pushLeftArrow()
{
  if (currentScreen == ActionListScreen)
  {
    actionIndex--;
    if (actionIndex < 0)
    {
      actionIndex = actionVector.size() - 1;
    }
    showActionScreen();
  }
}
void pushRightArrow()
{
  if (currentScreen == ActionListScreen)
  {
    actionIndex++;
    if (actionIndex >= actionVector.size())
    {
      actionIndex = 0;
    }
    showActionScreen();
  }
}
void pushUpArrow()
{
  if (currentScreen == ActionListScreen)
  {
    actionIndex--;
    if (actionIndex < 0)
    {
      actionIndex = actionVector.size() - 1;
    }
    showActionScreen();
  }
}
void pushDownArrow()
{
  if (currentScreen == ActionListScreen)
  {
    actionIndex++;
    if (actionIndex >= actionVector.size())
    {
      actionIndex = 0;
    }
    showActionScreen();
  }
}

// Reset state of kanji conversion
void resetKanjiConversion()
{
  kanjiVector.clear();
  kanjiIndex = -1;
}

// Load last file (Not in use)
/*
void loadLastFile()
{
  String fileName = textFileName(false);
  File textFile = SD.open(fileName.c_str(), FILE_READ);
  if (textFile && textFile.available())
  {
    String content = textFile.readString();
    updateScreen(content);
    textFile.close();
  }
}
*/
