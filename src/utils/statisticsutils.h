/*
 * Utils.h
 *
 *  Created on: 5 de dez de 2016
 *      Author: leo
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <QString>
#include <QMutex>
#include <string>
#include <vector>
#include <iterator>
//#include <cstdio>
//#include <cstdlib>
//#include <fstream>
#include <algorithm>
#include <cmath>
//#include <ctime>
//#include <tuple>
//#include <utility>
//#include <unordered_map>
//#include <iomanip>
//#include <libconfig.h++>

namespace utils{

namespace statistics {
    template<class T1, class T2, class T3, class T4>
    inline double calcula_desvio(const T1* vet, T2 ini, T3 fim, T4 media){
        double sum = 0.0;
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            sum += pow((vet[i] - media), 2.0);
        }

        sum = sum / (fim - ini);
        sum = sqrt(sum);

        return sum;
    }

    template<class T1, class T2, class T3>
    inline double calcula_media(const T1* vet, T2 ini, T3 fim){
        double sum = 0.0;
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            sum += vet[i];
        }
        return sum / (fim - ini);
    }

    template<class T1, class T2, class T3, class T4>
    inline double calcula_desvio(double (*f)(T1), const T1* vet, T2 ini, T3 fim, T4 media){
        double sum = 0.0;
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            sum += pow(((*f)(vet[i]) - media), 2.0);
        }

        sum = sum / (fim - ini);
        sum = sqrt(sum);

        return sum;
    }

    template<class T1, class T2, class T3>
    inline double calcula_media(double (*f)(T1), const T1* vet, T2 ini, T3 fim){
        double sum = 0.0;
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            sum += (*f)(vet[i]);
        }
        return sum / (fim - ini);
    }

    template<class T1, class T2>
    inline double media_harmonica(T1 num1, T2 num2){
        return 2 * ((num1 * num2) / (num1 + num2));
    }
}
}

#endif /* UTILS_H_ */
