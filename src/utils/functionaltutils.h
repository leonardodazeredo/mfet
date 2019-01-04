#ifndef FUNCTIONALTUTILS_H
#define FUNCTIONALTUTILS_H


namespace utils{

namespace functional {
    template <typename Collection,typename Predicate>
    inline Collection filterNot(Collection const & col,Predicate predicate ) {
        auto returnIterator = std::remove_if(col.begin(),col.end(),predicate);
        col.erase(returnIterator,std::end(col));
        return col;
    }

    //--------------------------------------------------V

    template <typename Collection,typename unop>
    inline void for_each(Collection const & col, unop op){
        std::for_each(col.begin(),col.end(),op);
    }

    template <typename Collection,typename binop>
    inline Collection zip(Collection const & fc,Collection sc,binop op) {
        Collection zipped(fc.size());
        std::transform(fc.begin(),fc.end(),sc.begin(),zipped.begin(),op);
        return zipped;
    }

    template <class Iter, typename result, typename unop>
    inline result reduce(Iter start, Iter end, result value, unop op){
        return std::accumulate(start, end, value, op);
    }

    template <typename Collection, typename result, typename unop>
    inline result reduce(Collection const & col, result value, unop op){
        return std::accumulate(col.begin(), col.end(), value, op);
    }

    template <typename C,typename unop>
    inline C map(C const & container,unop op) {
        C mapped(container.size());
        std::transform(container.begin(),container.end(),mapped.begin(),op);
        return mapped;
    }

    template<typename C, typename P>
    inline C filter(C const & container, P pred){
        C filtered(container.size());
        auto it = copy_if(container.begin(), container.end(), filtered.begin(), pred);
        filtered.resize(std::distance(filtered.begin(), it));
        return filtered;
    }

    template <typename Iter,typename result>
    inline bool exists(Iter start, Iter end, result& value) {
        auto p = [&value](result v){return v == value;};
        auto exist = std::find_if(start, end, p);
        return exist != end;
    }

    template <typename Collection,typename Condition>
    inline bool exists(Collection const & col,Condition con) {
        auto exist = std::find_if(col.begin(),col.end(),con);
        return exist != col.end();
    }
}
}
#endif // FUNCTIONALTUTILS_H
