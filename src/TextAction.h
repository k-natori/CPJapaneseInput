#ifndef TEXTACTION_H_INCLUDE
#define TEXTACTION_H_INCLUDE

#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5GFX.h>

class TextAction
{
public:
    TextAction();
    virtual boolean enabled();
    virtual String title() = 0;
    virtual String message();
    virtual String cancelTitle();
    virtual String OKTitle();
    virtual boolean prepareAction(M5GFX &lcd);
    virtual boolean doAction(M5GFX &lcd, String string);
};

#endif

/*
TextAction is a virtual superclass. Subclasses can implement actual actions that handle the text.

Subclasses of TextAction class can override functions above for:

enabled()   :Return if the action can be enabled. Should be used to determine if action can be performed
title()     :Return text title of the action. Should be used in actions list
message()   :Return text shown during preparation step
cancelTitle()   :Retrun cancel button title. Not used yet
OKTitle()   :Return OK button title. If this is not empty, the process shows OK button and wait until pushed.
             Otherwise the process move to next step automatically
prepareAction() :Called before doAction(). If this function returns false, the action should be canceled.
doAction()  :Called to perform actual action. If this function returns false, the process will wait key push before moving to text input mode

*/