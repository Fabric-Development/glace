#include <glib-2.0/glib.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__SSSE3__)
#include <tmmintrin.h>
#elif defined(__SSE2__)
#include <emmintrin.h>
#endif
#if defined(__AVX2__)
#include <immintrin.h>
#endif

static inline void buffer_utils_bgrx_to_rgbx(unsigned char* restrict data, long pixels) {
#if defined(__AVX2__)
    // 8 pixels (32 bytes) per iteration
    long i = 0;
    long n = pixels & ~7UL;

    const __m256i shuffle_mask = _mm256_setr_epi8(
        2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15, 18, 17, 16, 19, 22, 21, 20, 23, 26, 25, 24, 27, 30, 29, 28, 31
    );

    for (; i < n; i += 8) {
        __m256i px = _mm256_loadu_si256((__m256i*)(data + i * 4));
        px = _mm256_shuffle_epi8(px, shuffle_mask);
        _mm256_storeu_si256((__m256i*)(data + i * 4), px);
    }

    // leftovers
    for (; i < pixels; i++) {
        unsigned char pixel = data[i];
        data[i] = (pixel & 0xFF00FF00) | ((pixel << 16) & 0x00FF0000) | ((pixel >> 16) & 0xFF);
    }
#elif defined(__SSSE3__)
    // 4 pixels (16 bytes) per iteration
    long i = 0;
    long n = pixels & ~3UL;

    const __m128i shuffle_mask = _mm_setr_epi8(
        2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15
    );

    for (; i < n; i += 4) {
        __m128i px = _mm_loadu_si128((__m128i*)(data + i * 4));
        px = _mm_shuffle_epi8(px, shuffle_mask);
        _mm_storeu_si128((__m128i*)(data + i * 4), px);
    }

    for (; i < pixels; i++) {
        unsigned char pixel = data[i];
        data[i] = (pixel & 0xFF00FF00) | ((pixel << 16) & 0x00FF0000) | ((pixel >> 16) & 0xFF);
    }
#elif defined(__SSE2__)
    // 4 pixels (16 bytes) per iteration (shift & mask)
    long i = 0;
    long n = pixels & ~3UL;

    __m128i mask_rb = _mm_set1_epi32(0x00FF00FF);  // R and B
    __m128i mask_gx = _mm_set1_epi32(0xFF00FF00);  // G and X

    for (; i < n; i += 4) {
        __m128i px = _mm_loadu_si128((__m128i*)(data + i * 4));

        __m128i rb = _mm_and_si128(px, mask_rb);
        __m128i gx = _mm_and_si128(px, mask_gx);

        __m128i r = _mm_slli_epi32(rb, 16);
        __m128i b = _mm_srli_epi32(rb, 16);

        __m128i swapped = _mm_or_si128(gx, _mm_or_si128(r, b));
        _mm_storeu_si128((__m128i*)(data + i * 4), swapped);
    }

    for (; i < pixels; i++) {
        unsigned char pixel = data[i];
        data[i] = (pixel & 0xFF00FF00) | ((pixel << 16) & 0x00FF0000) | ((pixel >> 16) & 0xFF);
    }

#else
    for (long i = 0; i < pixels; i++) {
        unsigned char pixel = data[i];

        data[i] = (pixel & 0xFF00FF00) | ((pixel << 16) & 0x00FF0000) | ((pixel >> 16) & 0xFF);
    }
#endif
}

static void buffer_utils_log_available_accelerator() {
    const char* prefix = "[INFO][BUFFER]";
#if defined(__AVX2__)
    g_debug("%s using AVX2 for buffer format conversion acceleration", prefix);
#elif defined(__SSSE3__)
    g_debug("%s using SSSE3 for buffer format conversion acceleration", prefix);
#elif defined(__SSE2__)
    g_debug("%s using SSE2 for buffer format conversion acceleration", prefix);
#else
    g_debug("%s compiled with no buffer conversion acceleration", prefix);
#endif
}