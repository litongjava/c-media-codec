#ifndef RESAMPLER_COMPAT_H
#define RESAMPLER_COMPAT_H

/* ---------------------------------------------------
 * Choose floating point implementation
 * --------------------------------------------------- */
#ifndef FLOATING_POINT
#define FLOATING_POINT
#endif


/* ---------------------------------------------------
 * EXPORT macro for shared library symbols
 * --------------------------------------------------- */
#ifndef EXPORT
#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
#endif


/* ---------------------------------------------------
 * INLINE compatibility
 * --------------------------------------------------- */
#ifndef INLINE
#if defined(_MSC_VER)
#define INLINE __inline
#else
#define INLINE inline
#endif
#endif


/* ---------------------------------------------------
 * restrict keyword compatibility
 * --------------------------------------------------- */
#ifndef RESTRICT
#if defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif
#endif


#endif