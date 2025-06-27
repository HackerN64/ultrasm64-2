#ifndef LIBPL2_TEXTURE_
#define LIBPL2_TEXTURE_

#ifdef __cplusplus
extern "C" {
#endif

#include "libpl2-stddef.h"

/*! @defgroup page_texture Texture Structs
 *
 * The structs on this page are provided for convenience and can be used for buffers that store textures.
 * They can be used for things outside of libpl2.
 *
 * @{
 */

/*! An optional convenience struct for storing a single RGBA16 texel */
typedef struct {
	unsigned short r: 5; /*!< The red component of the colour */
	unsigned short g: 5; /*!< The green component of the colour */
	unsigned short b: 5; /*!< The blue component of the colour */
	unsigned short a: 1; /*!< The alpha bit of the colour */
} lpl2_texel_rgba16;
lpl2_static_assert( sizeof( lpl2_texel_rgba16 ) == 2 );

/*! A convenience struct for storing a single RGBA32 texel */
typedef struct {
	unsigned char r; /*!< The red component of the colour */
	unsigned char g; /*!< The green component of the colour */
	unsigned char b; /*!< The blue component of the colour */
	unsigned char a; /*!< The alpha component of the colour */
} lpl2_texel_rgba32 __attribute__((aligned(4)));
lpl2_static_assert( sizeof( lpl2_texel_rgba32 ) == 4 );

/*! A convenience struct for storing a 16x16 RGBA16 texture. */
typedef union {
	lpl2_texel_rgba16 texels[256];
	lpl2_texel_rgba16 yx[16][16];
} lpl2_texture_16x16_rgba16 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_16x16_rgba16 ) == 512 );

/*! A convenience struct for storing a 32x32 RGBA16 texture. */
typedef union {
	lpl2_texel_rgba16 texels[1024];
	lpl2_texel_rgba16 yx[32][32];
} lpl2_texture_32x32_rgba16 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_32x32_rgba16 ) == 2048 );

/*! A convenience struct for storing a 64x32 RGBA16 texture. */
typedef union {
	lpl2_texel_rgba16 texels[2048];
	lpl2_texel_rgba16 yx[32][64];
} lpl2_texture_64x32_rgba16 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_64x32_rgba16 ) == 4096 );

/*! A convenience struct for storing a 16x16 RGBA32 texture. */
typedef union {
	lpl2_texel_rgba32 texels[256];
	lpl2_texel_rgba32 yx[16][16];
} lpl2_texture_16x16_rgba32 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_16x16_rgba32 ) == 1024 );

/*! A convenience struct for storing a 32x32 RGBA32 texture. */
typedef union {
	lpl2_texel_rgba32 texels[1024];
	lpl2_texel_rgba32 yx[32][32];
} lpl2_texture_32x32_rgba32 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_32x32_rgba32 ) == 4096 );

/*! A convenience struct for storing a 64x16 RGBA32 texture. */
typedef union {
	lpl2_texel_rgba32 texels[1024];
	lpl2_texel_rgba32 yx[16][64];
} lpl2_texture_64x16_rgba32 __attribute__((aligned(8)));
lpl2_static_assert( sizeof( lpl2_texture_64x16_rgba32 ) == 4096 );

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
