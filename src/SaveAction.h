#ifndef SAVEACTION_H_INCLUDE
#define SAVEACTION_H_INCLUDE
#include "TextAction.h"

class SaveAction : public TextAction
{
public:
    SaveAction();
    String title() override;
    boolean doAction(M5GFX &lcd, String string) override;
    static void saveStringAsNewFile(String string);
};

#endif