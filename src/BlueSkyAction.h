#ifndef BLUESKYACTION_H_INCLUDE
#define BLUESKYACTION_H_INCLUDE
#include "TextAction.h"
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

class BlueSkyAction : public TextAction
{
private:
    static String _pemFileName;
    static String _pem;
    static String _accountName;
    static String _appPassword;
    static String _did;
    static String _accessJwt;
public:
    BlueSkyAction();
     boolean enabled() override;
    String title() override;
    String message() override;
    boolean prepareAction(M5GFX &lcd) override;
    boolean doAction(M5GFX &lcd, String string) override;

    static void setPEMFileName(String pemFileName);
    static String getPem();
    static void setAccountName(String accountName);
    static void setAppPassword(String appPassword);
};

#endif