#ifndef QRACTION_H_INCLUDE
#define QRACTION_H_INCLUDE
#include "TextAction.h"

class QRAction : public TextAction
{
public:
    QRAction();
    String title() override;
    boolean doAction(M5GFX &lcd, String string) override;
};

#endif