/*

   Base64的由来:
   目前Base64已经成为网络上常见的传输8Bit字节代码的编码方式之一。
   在做支付系统时，系统之间的报文交互都需要使用Base64对明文进行转码，然后再进行签名或加密，之后再进行（或再次Base64）传输。
   那么，Base64到底起到什么作用呢？

   在参数传输的过程中经常遇到的一种情况：
   使用全英文的没问题，但一旦涉及到中文就会出现乱码情况。
   与此类似，网络上传输的字符并不全是可打印的字符，比如二进制文件、图片等。
   Base64的出现就是为了解决此问题，它是基于64个可打印的字符来表示二进制的数据的一种方法。

   电子邮件刚问世的时候，只能传输英文，但后来随着用户的增加，中文、日文等文字的用户也有需求，
   但这些字符并不能被服务器或网关有效处理，因此Base64就登场了。
   随之，Base64在URL、Cookie、网页传输少量二进制文件中也有相应的使用。


   Base64的编码原理:
   Base64的原理比较简单，每当我们使用Base64时都会先定义一个类似这样的数组：
   ['A', 'B', 'C', ... 'a', 'b', 'c', ... '0', '1', ... '+', '/']
   上面就是Base64的索引表，字符选用了"A-Z、a-z、0-9、+、/" 64个可打印字符，这是标准的Base64协议规定。
   在日常使用中我们还会看到“=”或“==”号出现在Base64的编码结果中，“=”在此是作为填充字符出现，

   具体转换步骤:
   第一步，将待转换的字符串每三个字节分为一组，每个字节占8bit，那么共有24个二进制位。
   第二步，将上面的24个二进制位每6个一组，共分为4组。
   第三步，在每组前面添加两个0，每组由6个变为8个二进制位，总共32个二进制位，即四个字节。
   第四步，根据Base64编码对照表（见下图）获得对应的值。


   从上面的步骤我们发现：
   Base64字符表中的字符原本用6个bit就可以表示，现在前面添加2个0，变为8个bit，会造成一定的浪费。
   因此，Base64编码之后的文本，要比原文大约三分之一。
   为什么使用3个字节一组呢？因为6和8的最小公倍数为24，三个字节正好24个二进制位，每6个bit位一组，恰好能够分为4组。

   位数不足情况:
   上面是按照三个字节来举例说明的，如果字节数不足三个，那么该如何处理？

   两个字节：两个字节共16个二进制位，依旧按照规则进行分组。
   此时总共16个二进制位，每6个一组，则第三组缺少2位，用0补齐，得到三个Base64编码，第四组完全没有数据则用“=”补上。
   一个字节：一个字节共8个二进制位，依旧按照规则进行分组。
   此时共8个二进制位，每6个一组，则第二组缺少4位，用0补齐，得到两个Base64编码，而后面两组没有对应数据，都用“=”补上。

   注意事项:
   大多数编码都是由字符串转化成二进制的过程，而Base64的编码则是从二进制转换为字符串。与常规恰恰相反，
   Base64编码主要用在传输、存储、表示二进制领域，不能算得上加密，只是无法直接看到明文。也可以通过打乱Base64编码来进行加密。
   中文有多种编码（比如：utf-8、gb2312、gbk等），不同编码对应Base64编码结果都不一样。
   延伸:
   上面我们已经看到了Base64就是用6位（2的6次幂就是64）表示字符，因此成为Base64。
   同理，Base32就是用5位，Base16就是用4位。大家可以按照上面的步骤进行演化一下。
*/


#include <stdint.h>
#include <stdlib.h>


static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

static const unsigned char base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
     54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 127, 127, 127, 127, 127
};

#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */

/*
 * Encode a buffer into base64 format
 */
int base64_encode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen )
{
    size_t i, n;
    int C1, C2, C3;
    unsigned char *p;

    if( slen == 0 ) {
        *olen = 0;
        return( 0 );
    }

    n = slen / 3 + ( slen % 3 != 0 );

    if( n > ( BASE64_SIZE_T_MAX - 1 ) / 4 ) {
        *olen = BASE64_SIZE_T_MAX;
        return -1;
    }

    n *= 4;    // 编码后的字符数

    if( ( dlen < n + 1 ) || ( NULL == dst ) ) {
    	/*This will be the buf size that you need to malloc when 	
    	your dst is NULL*/
        *olen = n + 1;
        return -2;
    }

    n = ( slen / 3 ) * 3;

    for( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if( i < slen )
    {
        C1 = *src++;
        C2 = ( ( i + 1 ) < slen ) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if( ( i + 1 ) < slen )
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *olen = p - dst;
    *p = 0;

    return( 0 );
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen )
{
    size_t i, n;
    uint32_t j, x;
    unsigned char *p;

    /* First pass: check for validity and get output length */
    for( i = n = j = 0; i < slen; i++ )
    {
        /* Skip spaces before checking for EOL */
        x = 0;
        while( i < slen && src[i] == ' ' ) {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if( i == slen )
            break;

        if( ( slen - i ) >= 2 &&
            src[i] == '\r' && src[i + 1] == '\n' )
            continue;

        if( src[i] == '\n' )
            continue;

        /* Space inside a line is an error */
        if( x != 0 )
            return( -1 );

        if( src[i] == '=' && ++j > 2 )
            return( -1 );

        if( src[i] > 127 || base64_dec_map[src[i]] == 127 )
            return( -1 );

        if( base64_dec_map[src[i]] < 64 && j != 0 )
            return( -1 );

        n++;
    }

    if( n == 0 ) {
        *olen = 0;
        return( 0 );
    }

    /* The following expression is to calculate the following formula without
     * risk of integer overflow in n:
     *     n = ( ( n * 6 ) + 7 ) >> 3;
     */
    n = ( 6 * ( n >> 3 ) ) + ( ( 6 * ( n & 0x7 ) + 7 ) >> 3 );
    n -= j;

    if( dst == NULL || dlen < n )
    {
    	/*This will be the buf size that you need to malloc when 	
    	your dst is NULL*/
        *olen = n;
        return( -2);
    }

   for( j = 3, n = x = 0, p = dst; i > 0; i--, src++ )
   {
        if( *src == '\r' || *src == '\n' || *src == ' ' )
            continue;

        j -= ( base64_dec_map[*src] == 64 );
        x  = ( x << 6 ) | ( base64_dec_map[*src] & 0x3F );

        if( ++n == 4 )
        {
            n = 0;
            if( j > 0 ) *p++ = (unsigned char)( x >> 16 );
            if( j > 1 ) *p++ = (unsigned char)( x >>  8 );
            if( j > 2 ) *p++ = (unsigned char)( x       );
        }
    }

    *olen = p - dst;

    return( 0 );
}



#if 0
static const unsigned char _base64_encode_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char _base64_decode_chars[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};


static inline char* base64_encode(const unsigned char* in_str, int in_len, char* out_str)
{
    unsigned char c1, c2, c3;
    int i = 0;
    int len = in_len;
    int index = 0;
    while ( i<len ) {
        c1 = in_str[i++];
        if ( i == len ) {
            out_str[index++] = _base64_encode_chars[ c1>>2 ];
            out_str[index++] = _base64_encode_chars[ (c1&0x3)<<4 ];
            out_str[index++] = '=';
            out_str[index++] = '=';
            break;
        }
        c2 = in_str[i++];
        if ( i == len ) {
            out_str[index++] = _base64_encode_chars[ c1>>2 ];
            out_str[index++] = _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
            out_str[index++] = _base64_encode_chars[ (c2&0xF)<<2 ];
            out_str[index++] = '=';
            break;
        }
        c3 = in_str[i++];
        out_str[index++] = _base64_encode_chars[ c1>>2 ];
        out_str[index++] = _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
        out_str[index++] = _base64_encode_chars[ ((c2&0xF)<<2) | ((c3&0xC0)>>6) ];
        out_str[index++] = _base64_encode_chars[ c3&0x3F ];
    }
    out_str[index] = 0;
    return out_str;
}

static inline int base64_decode(const unsigned char* in_base64, int in_len,  unsigned char* out_data, int max_out_data_len)
{
    unsigned char c1, c2, c3, c4;
    int i = 0;
    int len = in_len;
    int index = 0;
    while ( i<len) {
        do { c1 = _base64_decode_chars[in_base64[i++] ]; } while ( i<len && c1 == 0xff);
        if ( c1 == 0xff) break;
        do { c2 = _base64_decode_chars[in_base64[i++] ]; } while ( i<len && c2 == 0xff);
        if ( c2 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] = (char) ((c1<<2) | ((c2&0x30)>>4)) ;
        do {
            c3 = in_base64[i++];
            if ( c3 == 61 ){ out_data[index] = 0; return index; } // meet with "=", break
            c3 = _base64_decode_chars[c3 ];
        } while ( i<len && c3 == 0xff);
        if ( c3 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] =  (char) ( ((c2&0XF)<<4) | ((c3&0x3C)>>2) );
        do {
            c4 = in_base64[i++]; if ( c4 == 61 ) { out_data[index] = 0; return index; } // meet with "=", break
            c4 = _base64_decode_chars[c4 ];
        } while ( i<len && c4 == 0xff );
        if ( c4 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] =  (char) ( ((c3&0x03)<<6) | c4 );
    }
    out_data[index] = 0;
    return index;
}
#endif
