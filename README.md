# CPJapaneseInput
CPJapaneseInput is a sketch to demonstrate japanese input on M5Stack Cardputer.

## How to run
1. This sketch requires M5Paper and microSD card
2. Put "settings.txt" into microSD. In this settings file you have to specify following
3. (Optional) Put PEM file (root CA for BlueSky, should be bsky-social.pem) into microSD
4. Specify your WiFi, dictionary file name in "settings.txt"
5. (Optional) Specify your BlueSky account and app password in "settings.txt"
6. Build and transfer this project as PlatformIO project

## How it works
Once transfered and run, it displays text input area. You can type alphabet directly, or push Fn + Space to switch Japanese input mode.
You can show fuction list by pushing Fn + Enter to perform some actions such as save text to SD, transfer as QR code, and post to BlueSky.

## Dependencies
This PlatformIO project depends on following libraries:
- m5stack/M5GFX
- m5stack/M5Unified
- m5stack/M5Cardputer
