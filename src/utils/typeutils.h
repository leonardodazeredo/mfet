#ifndef TYPEUTILS_H
#define TYPEUTILS_H

//#define INSTANCEOF (const GRASP* s = dynamic_cast<const GRASP*>(gf))

namespace utils{

namespace type {

template<class I, class T>
inline bool instanceOf(I* instance){
    if(const T* i = dynamic_cast<const T*>(&instance)) {
        return true;
    }
    else{
        return false;
    }
}

template<class I, class T>
inline T* instanceOfAndReturn(I instance){
    if(const T* i = dynamic_cast<const T*>(&instance)) {
        return i;
    }
    else{
        return nullptr;
    }
}

}
}

#endif // TYPEUTILS_H
