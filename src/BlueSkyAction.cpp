#include "BlueSkyAction.h"
#include "TextAction.h"

#define authURL "https://bsky.social/xrpc/com.atproto.server.createSession"
#define postURL "https://bsky.social/xrpc/com.atproto.repo.createRecord"

String BlueSkyAction::_pemFileName = "";
String BlueSkyAction::_pem = "";
String BlueSkyAction::_accountName = "";
String BlueSkyAction::_appPassword = "";
String BlueSkyAction::_accessJwt = "";
String BlueSkyAction::_did = "";

String ISOdateFromHTTPDate(String httpDate);

BlueSkyAction::BlueSkyAction()
{
}

boolean BlueSkyAction::enabled() {
    return (_pemFileName.length() > 0 && _accountName.length() > 0 && _appPassword.length() > 0);
}
String BlueSkyAction::title()
{
    return "Post to BlueSky";
}
String BlueSkyAction::message() {
    return "Posting...";
}

boolean BlueSkyAction::prepareAction(M5GFX &lcd)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }
    // Load PEM file in SD card
    File pemFile = SD.open(_pemFileName.c_str());
    if (pemFile)
    {
        _pem = pemFile.readString();
        pemFile.close();
        return true;
    }
    return false;
}
boolean BlueSkyAction::doAction(M5GFX &lcd, String string)
{

    HTTPClient httpClient;
    httpClient.begin(authURL, _pem.c_str());
    const char *headerKeys[] = {"Date"};
    httpClient.collectHeaders(headerKeys, 1);
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.addHeader("Accept", "application/json");

    String payload = "{\n\"identifier\": \"" + _accountName + "\",\n\"password\": \"" + _appPassword + "\"\n}";

    int result = httpClient.POST(payload);
    lcd.setTextColor(BLACK);
    if (!result == HTTP_CODE_OK)
    {
        lcd.fillRect(0, 0, lcd.width(), lcd.height(), RED);
        lcd.setCursor(0, 2);
        lcd.printf("%d Auth Error\n", result);
        return false;
    }
    String dateString = httpClient.header("Date");
    dateString = ISOdateFromHTTPDate(dateString);

    // Parse JSON to get accessJwt
    WiFiClient *stream = httpClient.getStreamPtr();
    if (httpClient.connected())
    {
        String eventBlock = "";
        boolean loadingEvent = false;
        while (stream->available())
        {
            String word = stream->readStringUntil('\"');
            if (word == "did")
            {
                stream->readStringUntil('\"');
                word = stream->readStringUntil('\"');
                _did = word;
            }
            else if (word == "accessJwt")
            {
                stream->readStringUntil('\"');
                word = stream->readStringUntil('\"');
                _accessJwt = word;
            }
        }
    }
    httpClient.end();

    if (_accessJwt.length() == 0)
    {
        lcd.fillRect(0, 0, lcd.width(), lcd.height(), RED);
        lcd.setCursor(0, 2);
        lcd.printf("%d Parse Error\n", result);
        lcd.println(_did);
        lcd.println(_accessJwt);
        return false;
    }

    // Post
    HTTPClient httpClient2;
    httpClient2.begin(postURL, _pem.c_str());
    httpClient2.addHeader("Content-Type", "application/json");
    httpClient2.addHeader("Accept", "application/json");
    httpClient2.addHeader("Authorization", "Bearer " + _accessJwt);
    string.replace("\n", "\\n");
    string.replace("\"", "\\\"");
    String post = "{\n\"$type\": \"app.bsky.feed.post\",\n\"text\": \"" + string + "\",\n\"createdAt\": \"" + dateString + "\"\n}";
    payload = "{\n\"repo\": \"" + _did + "\",\n\"collection\": \"app.bsky.feed.post\",\n\"record\": " + post + "\n}";

    result = httpClient2.POST(payload);
    if (!result == HTTP_CODE_OK)
    {
        lcd.fillRect(0, 0, lcd.width(), lcd.height(), RED);
        lcd.setCursor(0, 2);
        lcd.printf("%d Post Error\n", result);
        return false;
    }
    lcd.fillRect(0, 0, lcd.width(), lcd.height(), CYAN);
    lcd.setCursor(0, 2);
    lcd.println("Posted!");
    return false;
}

void BlueSkyAction::setPEMFileName(String pemFileName)
{
    _pemFileName = pemFileName;
}
String BlueSkyAction::getPem()
{
    return _pem;
}

void BlueSkyAction::setAccountName(String accountName)
{
    _accountName = accountName;
}
void BlueSkyAction::setAppPassword(String appPassword)
{
    _appPassword = appPassword;
}

String ISOdateFromHTTPDate(String httpDate)
{
    // Mon, 07 Aug 2023 14:14:37 GMT
    // to
    // 2023-08-07T14:14:37Z

    const String monthStringArray = "JanFebMarAprMayJunJulAugSepOctNovDec";

    int day = httpDate.substring(5, 7).toInt();
    String monthString = httpDate.substring(8, 11);
    int month = monthStringArray.indexOf(monthString) / 3 + 1;
    int year = httpDate.substring(12, 16).toInt();
    int hour = httpDate.substring(17, 19).toInt();
    int minute = httpDate.substring(20, 22).toInt();
    int second = httpDate.substring(23, 25).toInt();

    char result[21];
    snprintf(result, 21, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             year, month, day, hour, minute, second);
    return String(result);
}