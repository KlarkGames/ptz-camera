#ifndef ARDUINOCOMMAND_H
#define ARDUINOCOMMAND_H
#include <bitset>
#include <map>
#include <QString>
#include <QPointer>
#include <QObject>

#include "utils.h"

class ArduinoCommand : public QObject
{
public:
    virtual std::bitset<4> start() = 0;
    virtual std::bitset<4> stop() = 0;
};

class LeftCommand : public ArduinoCommand
{
public:
    std::bitset<4> start() override {return 0b1100;}
    std::bitset<4> stop() override {return 0b0000;}
};

class RightCommand : public ArduinoCommand
{
public:
    std::bitset<4> start() override {return 0b0100;}
    std::bitset<4> stop() override {return 0b0000;}
};

class UpCommand : public ArduinoCommand
{
public:
    std::bitset<4> start() override {return 0b0011;}
    std::bitset<4> stop() override {return 0b0000;}
};

class DownCommand : public ArduinoCommand
{
public:
    std::bitset<4> start() override {return 0b0001;}
    std::bitset<4> stop() override {return 0b0000;}
};

class HoldCommand : public ArduinoCommand
{
public:
    std::bitset<4> start() override {return 0b0000;}
    std::bitset<4> stop() override {return 0b0000;}
};

QPointer<ArduinoCommand> stringToArduinoCommand(QString direction);
QPointer<ArduinoCommand> directionToArduinoCommand(Direction direction);

#endif // ARDUINOCOMMAND_H
