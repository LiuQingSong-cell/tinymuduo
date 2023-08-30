#include "Timestamp.h"


std::string Timestamp::toString() const
{
    char buf[32] = {0};
    tm* time_tm = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, sizeof buf, "%4d-%02d-%02d %02d:%02d:%02d", 
             time_tm->tm_year + 1900,
             time_tm->tm_mon + 1,
             time_tm->tm_mday,
             time_tm->tm_hour,
             time_tm->tm_min,
             time_tm->tm_sec);
    return buf;
}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}