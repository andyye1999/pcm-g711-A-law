/* g711.c   A-law and linear PCM conversions. */

#define SIGN_BIT   (0x80) /* Sign bit for a A-law byte. */
#define QUANT_MASK (0xf)  /* Quantization field mask. */
#define NSEGS      (8)    /* Number of A-law segments. */
#define SEG_SHIFT  (4)    /* Left shift for segment number. */
#define SEG_MASK   (0x70) /* Segment field mask. */
static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

/* copy from CCITT G.711 specifications */
static short search(short val, short *table, short size)
{
    short  i;
    
    for (i = 0; i < size; i++)
    {
        if (val <= *table++) return (i);
    }
    return (size);
}

/* linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law

*  Linear Input Code Compressed Code
* ------------------------ ---------
* 0000000wxyza   000wxyz
* 0000001wxyza   001wxyz
* 000001wxyzab   010wxyz
* 00001wxyzabc   011wxyz
* 0001wxyzabcd   100wxyz
* 001wxyzabcde   101wxyz
* 01wxyzabcdef   110wxyz
* 1wxyzabcdefg   111wxyz
*/
unsigned char linear2alaw(short pcm_val) /* 2's complement (16-bit range) */
{
    short  mask, seg;
    unsigned char aval;
    
    if (pcm_val >= 0)
    {
        mask = 0xD5;  /* sign (7th) bit = 1 */
    }
    else
    {
        mask = 0x55;  /* sign bit = 0 */
        pcm_val = -pcm_val - 8;
    }
    
    /* Convert the scaled magnitude to segment number. */
    seg = search(pcm_val, seg_end, 8);
    
    /* Combine the sign, segment, and quantization bits. */
    if (seg >= 8)  /* out of range, return maximum value. */
    {
        return (0x7F ^ mask);
    }
    else
    {
        aval = seg << SEG_SHIFT;
        if (seg < 2)     aval |= (pcm_val >> 4) & QUANT_MASK;
        else             aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
        
        return (aval ^ mask);
    }
}
/* alaw2linear() - Convert an A-law value to 16-bit linear PCM */
short alaw2linear(unsigned char a_val)
{
    short  t, seg;
    
    a_val ^= 0x55;
    t = (a_val & QUANT_MASK) << 4;
    seg = ((unsigned short)a_val & SEG_MASK) >> SEG_SHIFT;
    
    switch (seg)
    {
        case 0:  t += 8;  break;
        case 1:  t += 0x108;  break;
        default: t += 0x108;  t <<= seg - 1;
    }
    return ((a_val & SIGN_BIT) ? t : -t);
}