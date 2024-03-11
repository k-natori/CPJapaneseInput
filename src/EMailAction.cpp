#include "EMailAction.h"
#include "EMailSender.h"

String EMailAction::_accountName = "";
String EMailAction::_password = "";
String EMailAction::_receiver = "";
boolean EMailAction::_makeFirstLineSubject = false;

EMailAction::EMailAction()
{
}

boolean EMailAction::enabled()
{
    return (_accountName.length() > 0 && _password.length() > 0 && _receiver.length() > 0);
}
String EMailAction::title()
{
    return "Send E-mail";
}
String EMailAction::message()
{
    return "Sending E-mail...";
}

boolean EMailAction::doAction(M5GFX &lcd, String string)
{
    EMailSender sender = EMailSender(_accountName.c_str(), _password.c_str());
    EMailSender::EMailMessage message;
    if (_makeFirstLineSubject)
    {
        int lineLocation = string.indexOf("\n");
        if (lineLocation > 0)
        {
            String firstLine = string.substring(0, lineLocation);
            String otherLines = string.substring(lineLocation + 1);
            otherLines.replace("\n", "\r\n");

            message.subject = firstLine;
            message.message = otherLines;
        } else {
            message.subject = string;
            message.message = "\r\n";
        }
    }
    else
    {
        string.replace("\n", "\r\n");

        message.subject = "Mail from Cardputer";
        message.message = string;
    }

    EMailSender::Response resp = sender.send(_receiver, message);

    lcd.fillScreen(CYAN);
    lcd.setCursor(0, 2);
    lcd.println(resp.desc);
    return false;
}

void EMailAction::setAccountName(String accountName)
{
    _accountName = accountName;
}
void EMailAction::setPassword(String password)
{
    _password = password;
}
void EMailAction::setReceiver(String receiver)
{
    _receiver = receiver;
}
void EMailAction::setMakeFirstLineSubject(boolean makeFirstLineSubject)
{
    _makeFirstLineSubject = makeFirstLineSubject;
}