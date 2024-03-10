#include "TextAction.h"

TextAction::TextAction() {
}
/*
String TextAction::title() {
    return "Action";
}
*/
boolean TextAction::enabled() {
    return true;
}
String TextAction::message() {
    return "";
}
String TextAction::cancelTitle() {
    return "";
}
String TextAction::OKTitle() {
    return "";
}
boolean TextAction::prepareAction(M5GFX &lcd) {
    return true;
}
boolean TextAction::doAction(M5GFX &lcd, String string) {
    return true;
}
