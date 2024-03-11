#ifndef EMAILACTION_H_INCLUDE
#define EMAILACTION_H_INCLUDE
#include "TextAction.h"

class EMailAction : public TextAction
{
private:
    static String _accountName;
    static String _password;
    static String _receiver;
    static boolean _makeFirstLineSubject;
public:
    EMailAction();
     boolean enabled() override;
    String title() override;
    String message() override;
    // boolean prepareAction(M5GFX &lcd) override;
    boolean doAction(M5GFX &lcd, String string) override;

    static void setAccountName(String accountName);
    static void setPassword(String password);
    static void setReceiver(String receiver);
    static void setMakeFirstLineSubject(boolean makeFirstLineSubject);
};

#endif