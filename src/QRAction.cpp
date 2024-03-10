#include "QRAction.h"

QRAction::QRAction() {

}

String QRAction::title() {
    return "QR code";
}

boolean QRAction::doAction(M5GFX &lcd, String string) {
    lcd.fillScreen(BLACK);
    lcd.qrcode(string, 0, 0, lcd.height());
    return false;
}
