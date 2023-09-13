#include "arduinocommand.h"

QPointer<ArduinoCommand> stringToArduinoCommand(QString direction)
{
    static std::map<QString, QPointer<ArduinoCommand>> stringToCommand = {
        {"up", QPointer<ArduinoCommand>(new UpCommand)},
        {"down", QPointer<ArduinoCommand>(new DownCommand)},
        {"left", QPointer<ArduinoCommand>(new LeftCommand)},
        {"right", QPointer<ArduinoCommand>(new RightCommand)}
    };

    return stringToCommand[direction.toLower()];
}

QPointer<ArduinoCommand> directionToArduinoCommand(Direction direction)
{
    static std::map<Direction, QPointer<ArduinoCommand>> directionToCommand = {
        {Direction::top, QPointer<ArduinoCommand>(new UpCommand)},
        {Direction::bottom, QPointer<ArduinoCommand>(new DownCommand)},
        {Direction::left, QPointer<ArduinoCommand>(new LeftCommand)},
        {Direction::right, QPointer<ArduinoCommand>(new RightCommand)},
        {Direction::hold, QPointer<ArduinoCommand>(new HoldCommand)}
    };

    return directionToCommand[direction];
}
