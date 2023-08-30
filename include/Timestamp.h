#ifndef TIMESTAMP__H__
#define TIMESTAMP__H__
#include <iostream>
#include <string>
#include <ctime>

class Timestamp
{
    public:
        explicit Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch){}
        Timestamp(): microSecondsSinceEpoch_(0){}

        std::string toString() const;
        static Timestamp now();
    
    private:
        int64_t microSecondsSinceEpoch_;
};



#endif