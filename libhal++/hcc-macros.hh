#ifndef _LIBHAL_CC_MACROS_HH_
#define _LIBHAL_CC_MACROS_HH_

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _HCC_BOOLEAN_EXPR(expr)             \
 __extension__ ({                           \
   int _boolean_var;                        \
   if (expr)                                \
      _boolean_var = 1;                     \
   else                                     \
      _boolean_var = 0;                     \
   _boolean_var;                            \
})
#define HCC_LIKELY(expr) (__builtin_expect (_HCC_BOOLEAN_EXPR(expr), 1))
#define HCC_UNLIKELY(expr) (__builtin_expect (_HCC_BOOLEAN_EXPR(expr), 0))
#else
#define HCC_LIKELY(expr) (expr)
#define HCC_UNLIKELY(expr) (expr)
#endif

#endif //!_LIBHAL_CC_MACROS_HH_

