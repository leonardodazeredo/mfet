#ifndef DECIMALUTILS_H
#define DECIMALUTILS_H

namespace utils{

namespace decimal {
    template<class floatOrDouble>
    inline floatOrDouble round_down(floatOrDouble number, unsigned places){

        floatOrDouble off = std::pow(10,places);

        floatOrDouble rounded_down = floor(number * off) / off;

        return rounded_down;
    }

    template<class floatOrDouble>
    inline floatOrDouble round_nearest(floatOrDouble number, unsigned places){

        floatOrDouble off = std::pow(10,places);

        floatOrDouble nearest = round(number * off) / off;

        return nearest;
    }

    template<class floatOrDouble>
    inline floatOrDouble round_up(floatOrDouble number, unsigned places){

        floatOrDouble off = std::pow(10,places);

        floatOrDouble rounded_up = ceil(number * off) / off;

        return rounded_up;
    }
}
}
#endif // DECIMALUTILS_H
