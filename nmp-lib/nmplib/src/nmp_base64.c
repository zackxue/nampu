#include "nmp_mem.h"
#include "nmp_base64.h"



void base64_free(void *data, size_t size)
{
	if (data && size>0)
	{
		//memset(data, 0, size);
		nmp_dealloc(data, size);
	}
}

static unsigned char *base64_malloc(size_t size)
{
	return (unsigned char *)nmp_alloc0(size);

	//static unsigned char base64[12*1024] = {0};
	//return base64;
}


static void decodeQuantum(unsigned char *dest, unsigned char *src)
{
  unsigned int x = 0;
  int i;
  for(i = 0; i < 4; i++) {
    if(src[i] >= 'A' && src[i] <= 'Z')
      x = (x << 6) + (unsigned int)(src[i] - 'A' + 0);
    else if(src[i] >= 'a' && src[i] <= 'z')
      x = (x << 6) + (unsigned int)(src[i] - 'a' + 26);
    else if(src[i] >= '0' && src[i] <= '9')
      x = (x << 6) + (unsigned int)(src[i] - '0' + 52);
    else if(src[i] == '+')
      x = (x << 6) + 62;
    else if(src[i] == '/')
      x = (x << 6) + 63;
    else if(src[i] == '=')
      x = (x << 6);
  }

  dest[2] = (unsigned char)(x & 255);
  x >>= 8;
  dest[1] = (unsigned char)(x & 255);
  x >>= 8;
  dest[0] = (unsigned char)(x & 255);
}

int base64_decode(unsigned char *data, unsigned int data_len, 
		unsigned char **dec_data, unsigned int *dec_data_len)
{
  int equalsTerm = 0;
  int i;
  int output_size;
  int numQuantums;
  unsigned char lastQuantum[3];
  unsigned char *newstr;

  if(data_len % 4 != 0)
  {
	  return -1;
  }

  /* Don't allocate a buffer if the decoded length is 0 */
  numQuantums = data_len / 4;
  if (numQuantums <= 0)
    return -1;

  /* A maximum of two = padding characters is allowed */
  if(data[data_len - 1] == '=') 
  {
    equalsTerm++;
    if(data[data_len - 2] == '=')
      equalsTerm++;
  }  

  *dec_data_len = (numQuantums * 3) - equalsTerm;

  /* The buffer must be large enough to make room for the last quantum
  (which may be partially thrown out) and the zero terminator. */
  output_size = *dec_data_len+4;
  *dec_data = newstr = base64_malloc(output_size);
  if(newstr == NULL)
    return -1;

  /* Decode all but the last quantum (which may not decode to a
  multiple of 3 bytes) */
  for(i = 0; i < numQuantums - 1; i++) {
    decodeQuantum((unsigned char *)newstr, data);
    newstr += 3; data += 4;
  }

  /* This final decode may actually read slightly past the end of the buffer
  if the input string is missing pad bytes.  This will almost always be
  harmless. */
  decodeQuantum(lastQuantum, data);
  for(i = 0; i < 3 - equalsTerm; i++)
    newstr[i] = lastQuantum[i];

  newstr[i] = 0; /* zero terminate */

  return output_size;
}

static const char table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(unsigned char *data, unsigned int data_len, 
		unsigned char **enc_data, unsigned int *enc_data_len)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;
  int output_size;
  unsigned char *output;

  //unsigned char *indata = data;
  
  output_size = data_len*4/3+4;
  *enc_data = output = base64_malloc(output_size);
  if(NULL == output)
    return -1;

  while(data_len > 0)
  {
    for (i = inputparts = 0; i < 3; i++) 
	{
      if(data_len > 0) 
	  {
        inputparts++;
        ibuf[i] = *data;
        data++;
        data_len--;
      }
      else
        ibuf[i] = 0;
    }

    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;

    switch(inputparts) {
    case 1: /* only one byte read */
      snprintf((char*)output, 5, "%c%c==",
               table64[obuf[0]],
               table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      snprintf((char*)output, 5, "%c%c%c=",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]]);
      break;
    default:
      snprintf((char*)output, 5, "%c%c%c%c",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]],
               table64[obuf[3]]);
      break;
    }
    output += 4;
  }
  *output=0;
  *enc_data_len = strlen((char*)*enc_data);

  return output_size; /* return the length of the new data */
}


