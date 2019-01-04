#ifndef LISTUTILS_H
#define LISTUTILS_H

#include <iostream>

#include "functionaltutils.h"

#include <vector>
#include <iostream>
#include <iterator>

using namespace std;

namespace utils{

namespace lists {
    template <typename Iter, typename result>
    inline result sum(Iter start, Iter end, result initialValue){
        return utils::functional::reduce(start, end, initialValue, [](result a, result b){return a + b;});
    }

    template <typename Collection, typename result>
    inline result sum(Collection const & col, result initialValue){
        return utils::functional::reduce(col, initialValue, [](result a, result b){return a + b;});
    }

    template <typename Iter, typename result>
    inline result prod(Iter start, Iter end, result initialValue){
        return utils::functional::reduce(start, end, initialValue, [](result a, result b){return a * b;});
    }

    template <typename Collection, typename result>
    inline result prod(Collection const & col, result initialValue){
        return utils::functional::reduce(col, initialValue, [](result a, result b){return a * b;});
    }

    template <typename Collection, typename valType>
    inline void values(Collection& col, valType value){
        for (auto &e: col){
            e = value;
        }
    }

    template <typename Collection>
    inline void ones(Collection& col){
        utils::lists::values(col, 1.0);
    }

    template <typename Collection>
    inline void zeros(Collection& col){
        utils::lists::values(col, 0.0);
    }

    template<class T1, class T2, class T3>
    inline void printVet(const T1* vet, T2 ini, T3 fim){
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            cout << i << " - " << vet[i] << endl;
        }
    }

    template<class T1, class T2, class T3, class A>
    inline void printVet(const T1* vet, T2 ini, T3 fim, A access){
        for (size_t i = ini; i < (size_t) fim; i++)
        {
            cout << i << " - " << access(vet[i]) << endl;
        }
    }

    // Cartesion product of vector of vectors
    // recursive algorithm to to produce cart. prod.
    // At any given moment, "me" points to some T1 in the middle of the
    // input data set.
    //   for int i in *me:
    //      add i to current result
    //      recurse on next "me"
    //
    template<class T1, class T2>
    void cart_product(
        T2& rvvi,  // final result
        T1&  rvi,   // current result
        typename T2::const_iterator me, // current input
        typename T2::const_iterator end) // final input
    {
        if(me == end) {
            // terminal condition of the recursion. We no longer have
            // any input vectors to manipulate. Add the current result (rvi)
            // to the total set of results (rvvvi).
            rvvi.push_back(rvi);
            return;
        }

        // need an easy name for my vector-of-ints
        const T1& mevi = *me;
        for(typename T1::const_iterator it = mevi.begin();
            it != mevi.end();
            it++) {
            // final rvi will look like "a, b, c, ME, d, e, f"
            // At the moment, rvi already has "a, b, c"
            rvi.push_back(*it);  // add ME
            cart_product(rvvi, rvi, me+1, end); //add "d, e, f"
            rvi.pop_back(); // clean ME off for next round
        }
    }
}
}
#endif // LISTUTILS_H
