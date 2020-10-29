#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds)
{
    long hours = seconds / 3600;
    seconds -= (hours * 3600);
    long minutes = seconds / 60;
    seconds -= (minutes * 60);

    char buffer[64];
    int length = sprintf(buffer, "%li:%02li:%02li", hours, minutes, seconds);
    buffer[length] = 0; //null-terminate the string

    return std::string(buffer);
}