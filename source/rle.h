#ifndef __RLE_H__
#define __RLE_H__

int rle_encode_u8(int len, unsigned char* bytes, unsigned char* out_buffer)
{
    unsigned char * buffer_start = out_buffer;
    unsigned char * bytes_end = bytes + len;
    while(bytes != bytes_end)
    {
        unsigned char c = 0;
        unsigned char b = *bytes;
        while(*bytes == b && c < 255)
        {
            ++bytes;
            ++c;
        }
        *out_buffer++ = c;
        *out_buffer++ = b;
    }
    *out_buffer++ = 0;
    return out_buffer - buffer_start;
}

typedef void(*rle_decode_u8_callback)(unsigned char);

void rle_decode_u8(unsigned char* bytes, rle_decode_u8_callback callback)
{
    while(*bytes != 0)
    {
        unsigned char c = *bytes++;
        unsigned char b = *bytes++;
        while(c--)
            callback(b);
    }
}

int rle_encode_u16(int len, unsigned short* bytes, unsigned short* out_buffer)
{
    unsigned short * buffer_start = out_buffer;
    unsigned short * bytes_end = bytes + len;
    while(bytes != bytes_end)
    {
        unsigned char c = 0;
        unsigned char b = *bytes;
        while(*bytes == b && c < 255)
        {
            ++bytes;
            ++c;
        }
        *out_buffer++ = c;
        *out_buffer++ = b;
    }
    *out_buffer++ = 0;
    return out_buffer - buffer_start;
}

typedef void(*rle_decode_u16_callback)(unsigned short);

void rle_decode_u16(unsigned short* bytes, rle_decode_u16_callback callback)
{
    while(*bytes != 0)
    {
        unsigned short c = *bytes++;
        unsigned short b = *bytes++;
        while(c--)
            callback(b);
    }
}

#endif //__RLE_H__
