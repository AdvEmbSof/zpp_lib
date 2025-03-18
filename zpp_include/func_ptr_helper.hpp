namespace zpp_lib {
  
template <const size_t UniqueId, typename Res, typename... ArgTypes>
class FuncPtrHelper
{
public:
    typedef std::function<Res(ArgTypes...)> FunctionType;

    static void bind(FunctionType&& f) { 
      instance()._fn.swap(f); 
    }

    static void bind(const FunctionType& f) { 
      instance()._fn = f; 
    }

    static Res invoke(ArgTypes... args) { 
      return instance()._fn(args...); 
    }

    typedef decltype(&FuncPtrHelper::invoke) PointerType;
    static PointerType ptr() { 
      return &invoke; 
    }

private:
    static FuncPtrHelper& instance() {
        static FuncPtrHelper instance;
        return instance;
    }

    FunctionType _fn;
};

template <const size_t UniqueId, typename Res, typename... ArgTypes>
typename FuncPtrHelper<UniqueId, Res, ArgTypes...>::PointerType
getFuncPtr(const typename FuncPtrHelper<UniqueId, Res, ArgTypes...>::FunctionType& f) {
    FuncPtrHelper<UniqueId, Res, ArgTypes...>::bind(f);
    return FuncPtrHelper<UniqueId, Res, ArgTypes...>::ptr();
}

} // namespace zpp_lib