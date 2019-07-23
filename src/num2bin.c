/* convert to/from cryptic integer encoding */

short int num2bin(short int);
short int bin2num(short int);

short int num2bin(short int num)
{
	short int bin = 0;
	char *pbin = NULL;
	pbin = (char *) &bin;

	// handle zero
	if (num == 0)
		return bin;

	// handle sign
	unsigned char sign = 0;
	if (num < 0)
	{
		sign = 1;
		num = -num;
	}

	// find first nonzero bit, 'lsb 0' bit numbering
	// num != 0 here
	char firstBit;
	for (firstBit=7; firstBit>=0; firstBit--)
	{
		if (((num >> firstBit) & 1) == 1)
			break;
	}
	
	// construct result:
	// bit 0    : sign bit
	// bit 1-8  : firstBit + 127
	// bit 9-15 : num without firstBit, left aligned
	pbin[0] = (sign << 7) + ((firstBit + 127) >> 1);
	pbin[1] = (((firstBit + 127) & 1) << 7) + ((num - (1 << firstBit)) << (7-firstBit));

	return bin;
}

short int bin2num(short int bin)
{
	char *pbin = NULL;
	pbin = (char *) &bin;

	// zero
	if (bin == 0)
		return 0;

	// sign
	unsigned char sign = 0;
	sign = (pbin[0] >> 7) & 1;
	
	// num length
	char firstBit;
	firstBit = ((pbin[0] & 127) << 1) + ((pbin[1] & 128) >> 7) - 127;
	
	// result
	short int num;
	num = ((pbin[1] & 127) >> (7-firstBit)) + (1 << firstBit);

	// sign
	if (sign == 1)
		num = -num;

	return num;
}
