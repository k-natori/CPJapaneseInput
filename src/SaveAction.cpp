#include "SaveAction.h"
#include <SD.h>

#define baseFileName "/note"

// determine file name
String textFileName(boolean next)
{
    int fileCount = 0;
    String fileName = baseFileName + String(fileCount) + ".txt";
    while (SD.exists(fileName))
    {
        fileCount++;
        fileName = baseFileName + String(fileCount) + ".txt";
    }
    if (next)
    {
        return fileName;
    }
    if (fileCount > 0)
        fileName = baseFileName + String(fileCount - 1) + ".txt";
    return fileName;
}

SaveAction::SaveAction() {
    
}

String SaveAction::title() {
    return "Save to SD";
}

boolean SaveAction::doAction(M5GFX &lcd, String string)
{
    String fileName = textFileName(true);
    File textFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (textFile)
    {
        textFile.print(string);
        textFile.close();
        return true;
    }
    return false;
}

void SaveAction::saveStringAsNewFile(String string) {
    String fileName = textFileName(true);
    File textFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (textFile)
    {
        textFile.print(string);
        textFile.close();
    }
}
