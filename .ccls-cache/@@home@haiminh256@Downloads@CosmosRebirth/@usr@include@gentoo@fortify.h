#ifdef __clang__
# pragma clang system_header
#endif
#ifndef _FORTIFY_SOURCE
# if defined(__has_feature)
#  define __GENTOO_HAS_FEATURE(x) __has_feature(x)
# else
#  define __GENTOO_HAS_FEATURE(x) 0
# endif
#
# if !defined(__OPTIMIZE__) || __OPTIMIZE__ == 0
# elif !defined(__STDC_HOSTED__) || __STDC_HOSTED__ != 1
# elif defined(__SANITIZE_ADDRESS__)
# elif __GENTOO_HAS_FEATURE(address_sanitizer)
# elif __GENTOO_HAS_FEATURE(hwaddress_sanitizer)
# elif __GENTOO_HAS_FEATURE(memory_sanitizer)
# elif __GENTOO_HAS_FEATURE(numerical_stability_sanitizer)
# elif __GENTOO_HAS_FEATURE(realtime_sanitizer)
# elif __GENTOO_HAS_FEATURE(thread_sanitizer)
# else
#  define _FORTIFY_SOURCE 2
# endif
#
# undef __GENTOO_HAS_FEATURE
#endif
