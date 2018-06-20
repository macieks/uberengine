#include "Base/ueEncoding.h"
#include "ctype.h"

static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

u32 ueEncoding_EstimateBase64(u32 len)
{
	u32 olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
    olen++; /* null termination */
	return olen;
}

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
ueBool ueEncoding_EncodeBase64(const char* src, u32 len, char* dst, u32 dstCapacity, u32* dstLength)
{
	char *out, *pos;
	const char *end, *in;
	size_t olen;
#ifdef BASE64_ENCODE_ADD_NEWLINES
	int line_len;
#endif

	olen = ueEncoding_EstimateBase64(len);

	if (dstCapacity < olen)
		return UE_FALSE;
	out = dst;

	end = src + len;
	in = src;
	pos = out;
#ifdef BASE64_ENCODE_ADD_NEWLINES
	line_len = 0;
#endif

	while (end - in >= 3)
	{
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
#ifdef BASE64_ENCODE_ADD_NEWLINES
		line_len += 4;
		if (line_len >= 72)
		{
			*pos++ = '\n';
			line_len = 0;
		}
#endif
	}

	if (end - in)
	{
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1)
		{
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else
		{
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
								  (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
#ifdef BASE64_ENCODE_ADD_NEWLINES
		line_len += 4;
#endif
	}

#ifdef BASE64_ENCODE_ADD_NEWLINES
	if (line_len)
		*pos++ = '\n';
#endif

	*pos = '\0';

	if (dstLength)
		*dstLength = pos - out;
	return UE_TRUE;
}

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

u32 ueEncoding_EstimateUrl(const char* src)
{
	return ueStrLen(src) * 3 + 1;
}

ueBool ueEncoding_EncodeUrl(const char* str, char* dst, u32 dstCapacity)
{
	if (dstCapacity < ueEncoding_EstimateUrl(str))
		return UE_FALSE;

  const char *pstr = str;
  char *buf = dst;
  char *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return UE_TRUE;
}

char* ueEncoding_DecodeUrl(char *str) {
  char *pstr = str, *buf = (char* ) malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
