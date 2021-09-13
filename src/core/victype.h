#ifndef __VICTYPE_H__
#define __VICTYPE_H__

#include "../port.h"

#define VI_CTF_LOWER  0x01
#define VI_CTF_UPPER  0x02
#define VI_CTF_ALPHA  (VI_CTF_LOWER|VI_CTF_UPPER)
#define VI_CTF_DIGIT  0x04
#define VI_CTF_ALNUM  (VI_CTF_ALPHA|VI_CTF_DIGIT)
#define VI_CTF_SPACE  0x08
#define VI_CTF_XDIGIT 0x10

extern const unsigned int Vi_ctype_table[256];

/* Unlike their C counterparts, the following macros are not meant to
 * handle an int with any of the values [EOF, 0-UCHAR_MAX]. The argument
 * must be a signed/unsigned char. */
#define Vi_ISLOWER(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_LOWER)
#define Vi_ISUPPER(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_UPPER)
#define Vi_ISALPHA(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_ALPHA)
#define Vi_ISDIGIT(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_DIGIT)
#define Vi_ISXDIGIT(c) (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_XDIGIT)
#define Vi_ISALNUM(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_ALNUM)
#define Vi_ISSPACE(c)  (Vi_ctype_table[VI_CHARMASK(c)] & VI_CTF_SPACE)

extern const unsigned char Vi_ctype_tolower[256];
extern const unsigned char Vi_ctype_toupper[256];

#define Vi_TOLOWER(c) (Vi_ctype_tolower[VI_CHARMASK(c)])
#define Vi_TOUPPER(c) (Vi_ctype_toupper[VI_CHARMASK(c)])

#endif // __VICTYPE_H__
