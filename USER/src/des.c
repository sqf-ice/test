//#include "Rc531.h"
#include "des.h"
#include <stdio.h>
#include <string.h>
#define uchar unsigned char
#define uint  unsigned int




uchar   oo[4]={0,0,};
uchar   pp[4]={0,0,};
uchar   fnc[4]={0,0,0,0};
uchar  enckey[96]={0,0,};
uchar  deckey[96]={0,0,};




 const  uchar	
	S_BOX1[64]= {
		0xe,0x0,0x4,0xf,0xd,0x7,0x1,0x4,
		0x2,0xe,0xf,0x2,0xb,0xd,0x8,0x1,
		0x3,0xa,0xa,0x6,0x6,0xc,0xc,0xb,
		0x5,0x9,0x9,0x5,0x0,0x3,0x7,0x8,
		0x4,0xf,0x1,0xc,0xe,0x8,0x8,0x2,
		0xd,0x4,0x6,0x9,0x2,0x1,0xb,0x7,
		0xf,0x5,0xc,0xb,0x9,0x3,0x7,0xe,
		0x3,0xa,0xa,0x0,0x5,0x6,0x0,0xd,
	},

	S_BOX2[64]= {
		0xf,0x3,0x1,0xd,0x8,0x4,0xe,0x7,
		0x6,0xf,0xb,0x2,0x3,0x8,0x4,0xe,
		0x9,0xc,0x7,0x0,0x2,0x1,0xd,0xa,
		0xc,0x6,0x0,0x9,0x5,0xb,0xa,0x5,
		0x0,0xd,0xe,0x8,0x7,0xa,0xb,0x1,
		0xa,0x3,0x4,0xf,0xd,0x4,0x1,0x2,
		0x5,0xb,0x8,0x6,0xc,0x7,0x6,0xc,
		0x9,0x0,0x3,0x5,0x2,0xe,0xf,0x9,
	},

	S_BOX3[64]= {
		0xa,0xd,0x0,0x7,0x9,0x0,0xe,0x9,
		0x6,0x3,0x3,0x4,0xf,0x6,0x5,0xa,
		0x1,0x2,0xd,0x8,0xc,0x5,0x7,0xe,
		0xb,0xc,0x4,0xb,0x2,0xf,0x8,0x1,
		0xd,0x1,0x6,0xa,0x4,0xd,0x9,0x0,
		0x8,0x6,0xf,0x9,0x3,0x8,0x0,0x7,
		0xb,0x4,0x1,0xf,0x2,0xe,0xc,0x3,
		0x5,0xb,0xa,0x5,0xe,0x2,0x7,0xc,
	},

	S_BOX4[64]= {
		0x7,0xd,0xd,0x8,0xe,0xb,0x3,0x5,
		0x0,0x6,0x6,0xf,0x9,0x0,0xa,0x3,
		0x1,0x4,0x2,0x7,0x8,0x2,0x5,0xc,
		0xb,0x1,0xc,0xa,0x4,0xe,0xf,0x9,
		0xa,0x3,0x6,0xf,0x9,0x0,0x0,0x6,
		0xc,0xa,0xb,0x1,0x7,0xd,0xd,0x8,
		0xf,0x9,0x1,0x4,0x3,0x5,0xe,0xb,
		0x5,0xc,0x2,0x7,0x8,0x2,0x4,0xe,
	},

	S_BOX5[64]= {
		0x2,0xe,0xc,0xb,0x4,0x2,0x1,0xc,
		0x7,0x4,0xa,0x7,0xb,0xd,0x6,0x1,
		0x8,0x5,0x5,0x0,0x3,0xf,0xf,0xa,
		0xd,0x3,0x0,0x9,0xe,0x8,0x9,0x6,
		0x4,0xb,0x2,0x8,0x1,0xc,0xb,0x7,
		0xa,0x1,0xd,0xe,0x7,0x2,0x8,0xd,
		0xf,0x6,0x9,0xf,0xc,0x0,0x5,0x9,
		0x6,0xa,0x3,0x4,0x0,0x5,0xe,0x3,
	},

	S_BOX6[64]= {
		0xc,0xa,0x1,0xf,0xa,0x4,0xf,0x2,
		0x9,0x7,0x2,0xc,0x6,0x9,0x8,0x5,
		0x0,0x6,0xd,0x1,0x3,0xd,0x4,0xe,
		0xe,0x0,0x7,0xb,0x5,0x3,0xb,0x8,
		0x9,0x4,0xe,0x3,0xf,0x2,0x5,0xc,
		0x2,0x9,0x8,0x5,0xc,0xf,0x3,0xa,
		0x7,0xb,0x0,0xe,0x4,0x1,0xa,0x7,
		0x1,0x6,0xd,0x0,0xb,0x8,0x6,0xd,
	},

	S_BOX7[64]= {
		0x4,0xd,0xb,0x0,0x2,0xb,0xe,0x7,
		0xf,0x4,0x0,0x9,0x8,0x1,0xd,0xa,
		0x3,0xe,0xc,0x3,0x9,0x5,0x7,0xc,
		0x5,0x2,0xa,0xf,0x6,0x8,0x1,0x6,
		0x1,0x6,0x4,0xb,0xb,0xd,0xd,0x8,
		0xc,0x1,0x3,0x4,0x7,0xa,0xe,0x7,
		0xa,0x9,0xf,0x5,0x6,0x0,0x8,0xf,
		0x0,0xe,0x5,0x2,0x9,0x3,0x2,0xc,
	},

	S_BOX8[64]= {
		0xd,0x1,0x2,0xf,0x8,0xd,0x4,0x8,
		0x6,0xa,0xf,0x3,0xb,0x7,0x1,0x4,
		0xa,0xc,0x9,0x5,0x3,0x6,0xe,0xb,
		0x5,0x0,0x0,0xe,0xc,0x9,0x7,0x2,
		0x7,0x2,0xb,0x1,0x4,0xe,0x1,0x7,
		0x9,0x4,0xc,0xa,0xe,0x8,0x2,0xd,
		0x0,0xf,0x6,0xc,0xa,0x9,0xd,0x0,
		0xf,0x3,0x3,0x5,0x5,0x6,0x8,0xb,
	};

void SubKey_Generation(uchar *key)
{
	uchar   i,j;
	uchar   skey[6]={0,0,};
	uchar   ls[16]={1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
	uchar   lk[4]={0,0,0,0};
	uchar   rk[4]={0,0,0,0};

	memset (lk, 0, 4);
	memset (rk, 0, 4);

	// now PC1 permutation
	lk[3] = (lk[3] | (key[7] >> 7 & 0x01)) << 1;	//
	lk[3] = (lk[3] | (key[6] >> 7 & 0x01)) << 1;
	lk[3] = (lk[3] | (key[5] >> 7 & 0x01)) << 1;
	lk[3] = (lk[3] | (key[4] >> 7 & 0x01));

	lk[2] = (lk[2] | (key[3] >> 7 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[2] >> 7 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[1] >> 7 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[0] >> 7 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[7] >> 6 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[6] >> 6 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[5] >> 6 & 0x01)) << 1;
	lk[2] = (lk[2] | (key[4] >> 6 & 0x01));

	lk[1] = (lk[1] | (key[3] >> 6 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[2] >> 6 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[1] >> 6 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[0] >> 6 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[7] >> 5 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[6] >> 5 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[5] >> 5 & 0x01)) << 1;
	lk[1] = (lk[1] | (key[4] >> 5 & 0x01));

	lk[0] = (lk[0] | (key[3] >> 5 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[2] >> 5 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[1] >> 5 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[0] >> 5 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[7] >> 4 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[6] >> 4 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[5] >> 4 & 0x01)) << 1;
	lk[0] = (lk[0] | (key[4] >> 4 & 0x01));

	rk[3] = (rk[3] | (key[7] >> 1 & 0x01)) << 1;
	rk[3] = (rk[3] | (key[6] >> 1 & 0x01)) << 1;
	rk[3] = (rk[3] | (key[5] >> 1 & 0x01)) << 1;
	rk[3] = (rk[3] | (key[4] >> 1 & 0x01));

	rk[2] = (rk[2] | (key[3] >> 1 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[2] >> 1 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[1] >> 1 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[0] >> 1 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[7] >> 2 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[6] >> 2 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[5] >> 2 & 0x01)) << 1;
	rk[2] = (rk[2] | (key[4] >> 2 & 0x01));

	rk[1] = (rk[1] | (key[3] >> 2 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[2] >> 2 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[1] >> 2 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[0] >> 2 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[7] >> 3 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[6] >> 3 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[5] >> 3 & 0x01)) << 1;
	rk[1] = (rk[1] | (key[4] >> 3 & 0x01));

	rk[0] = (rk[0] | (key[3] >> 3 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[2] >> 3 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[1] >> 3 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[0] >> 3 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[3] >> 4 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[2] >> 4 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[1] >> 4 & 0x01)) << 1;
	rk[0] = (rk[0] | (key[0] >> 4 & 0x01));

	lk[3] &= 0x0F;
	rk[3] &= 0x0F;

	for(i=0; i<=16; i++)
	{
		j = 6*i;

		skey[5]=0;
		skey[4]=0;
		skey[3]=0;
		skey[2]=0;
		skey[1]=0;
		skey[0]=0;

		if(ls[i] == 1)
		{
			lk[3] = (lk[3] << 1 | lk[2] >> 7);
			lk[2] = (lk[2] << 1 | lk[1] >> 7);
			lk[1] = (lk[1] << 1 | lk[0] >> 7);
			lk[0] = (lk[0] << 1 | lk[3] >> 4);

			lk[3] &= 0x0F;

			rk[3] = (rk[3] << 1 | rk[2] >> 7);
			rk[2] = (rk[2] << 1 | rk[1] >> 7);
			rk[1] = (rk[1] << 1 | rk[0] >> 7);
			rk[0] = (rk[0] << 1 | rk[3] >> 4);

			rk[3] &= 0x0F;
		}
		else // ls[i] == 2
		{
			lk[3] = (lk[3] << 2 | lk[2] >> 6);
			lk[2] = (lk[2] << 2 | lk[1] >> 6);
			lk[1] = (lk[1] << 2 | lk[0] >> 6);
			lk[0] = (lk[0] << 2 | lk[3] >> 4);

			lk[3] &= 0x0F;

			rk[3] = (rk[3] << 2 | rk[2] >> 6);
			rk[2] = (rk[2] << 2 | rk[1] >> 6);
			rk[1] = (rk[1] << 2 | rk[0] >> 6);
			rk[0] = (rk[0] << 2 | rk[3] >> 4);

			rk[3] &= 0x0F;
		}


		// now PC2 permutation
		skey[5] = (skey[5] | (lk[1] >> 6 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[1] >> 3 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[2] >> 1 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[0] >> 4 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[3] >> 3 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[2] >> 7 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[3] >> 1 & 0x01)) << 1;
		skey[5] = (skey[5] | (lk[0]      & 0x01));

		enckey[j+5] = skey[5];

		skey[4] = (skey[4] | (lk[1] >> 5 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[2] >> 6 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[0] >> 7 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[2] >> 2 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[0] >> 5 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[1] >> 1 & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[2]      & 0x01)) << 1;
		skey[4] = (skey[4] | (lk[3]      & 0x01));

		enckey[j+4] = skey[4];

		skey[3] = (skey[3] | (lk[0] >> 2 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[2] >> 4 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[1] >> 4 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[2] >> 5 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[0] >> 1 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[1]      & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[1] >> 7 & 0x01)) << 1;
		skey[3] = (skey[3] | (lk[3] >> 2 & 0x01));

		enckey[j+3] = skey[3];

		skey[2] = (skey[2] | (rk[1] >> 7 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[0] >> 4 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[3] >> 1 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[2] >> 3 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[1] >> 1 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[0] >> 1 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[3] >> 2 & 0x01)) << 1;
		skey[2] = (skey[2] | (rk[2]      & 0x01));

		enckey[j+2] = skey[2];

		skey[1] = (skey[1] | (rk[0] >> 5 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[1] >> 3 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[2] >> 7 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[1]      & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[1] >> 4 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[0] >> 7 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[2] >> 1 & 0x01)) << 1;
		skey[1] = (skey[1] | (rk[0]      & 0x01));

		enckey[j+1] = skey[1];

		skey[0] = (skey[0] | (rk[2] >> 6 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[0] >> 3 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[1] >> 2 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[1] >> 6 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[0] >> 6 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[2] >> 4 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[3] >> 3 & 0x01)) << 1;
		skey[0] = (skey[0] | (rk[3]      & 0x01));

		enckey[j] = skey[0];
	}
 
	for(i=0;i<8;i++) // create decription subkeys...
	{
		memcpy(deckey+6*i, enckey+(90-6*i), 6);
		memcpy(deckey+(90-6*i), enckey+6*i, 6);
	}
}



void IniPer(uchar *src)
{
	memset(oo, 0, 4);
	memset(pp, 0, 4);

//  oo[3] = src[7].6 | src[6].6 | src[5].6 | src[4].6 | src[3].6 | src[2].6 | src[1].6 | src[0].6;
	oo[3] = (oo[3] | (src[7] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[6] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[5] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[4] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[3] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[2] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[1] >> 6 & 0x01)) << 1;
	oo[3] = (oo[3] | (src[0] >> 6 & 0x01));


//	oo[2] = src[7].4 | src[6].4 | src[5].4 | src[4].4 | src[3].4 | src[2].4 | src[1].4 | src[0].4;
	oo[2] = (oo[2] | (src[7] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[6] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[5] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[4] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[3] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[2] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[1] >> 4 & 0x01)) << 1;
	oo[2] = (oo[2] | (src[0] >> 4 & 0x01));

//	oo[1] = src[7].2 | src[6].2 | src[5].2 | src[4].2 | src[3].2 | src[2].2 | src[1].2 | src[0].2;
	oo[1] = (oo[1] | (src[7] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[6] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[5] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[4] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[3] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[2] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[1] >> 2 & 0x01)) << 1;
	oo[1] = (oo[1] | (src[0] >> 2 & 0x01));

//	oo[0] = src[7].0 | src[6].0 | src[5].0 | src[4].0 | src[3].0 | src[2].0 | src[1].0 | src[0].0;
	oo[0] = (oo[0] | (src[7]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[6]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[5]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[4]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[3]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[2]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[1]      & 0x01)) << 1;
	oo[0] = (oo[0] | (src[0]      & 0x01));

//	pp[3] = src[7].7 | src[6].7 | src[5].7 | src[4].7 | src[3].7 | src[2].7 | src[1].7 | src[0].7;
	pp[3] = (pp[3] | (src[7] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[6] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[5] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[4] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[3] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[2] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[1] >> 7 & 0x01)) << 1;
	pp[3] = (pp[3] | (src[0] >> 7 & 0x01));


//	pp[2] = src[7].5 | src[6].5 | src[5].5 | src[4].5 | src[3].5 | src[2].5 | src[1].5 | src[0].5;
	pp[2] = (pp[2] | (src[7] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[6] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[5] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[4] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[3] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[2] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[1] >> 5 & 0x01)) << 1;
	pp[2] = (pp[2] | (src[0] >> 5 & 0x01));


//	pp[1] = src[7].3 | src[6].3 | src[5].3 | src[4].3 | src[3].3 | src[2].3 | src[1].3 | src[0].3;
	pp[1] = (pp[1] | (src[7] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[6] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[5] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[4] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[3] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[2] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[1] >> 3 & 0x01)) << 1;
	pp[1] = (pp[1] | (src[0] >> 3 & 0x01));


//	pp[0] = src[7].1 | src[6].1 | src[5].1 | src[4].1 | src[3].1 | src[2].1 | src[1].1 | src[0].1;
	pp[0] = (pp[0] | (src[7] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[6] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[5] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[4] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[3] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[2] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[1] >> 1 & 0x01)) << 1;
	pp[0] = (pp[0] | (src[0] >> 1 & 0x01));
}


void desfun(uchar *subkey)
{
	uchar   blck[9]={0,0,0,};
	memset(blck, 0, 9);

	fnc[0] = 0;
	fnc[1] = 0;
	fnc[2] = 0;
	fnc[3] = 0;

	blck[8] = (pp[0] & 0x0F) << 1;
	blck[7] = (pp[0] >> 4 & 0x0F) << 1;
	blck[6] = (pp[1] & 0x0F) << 1;
	blck[5] = (pp[1] >> 4 & 0x0F) << 1;
	blck[4] = (pp[2] & 0x0F) << 1;
	blck[3] = (pp[2] >> 4 & 0x0F) << 1;
	blck[2] = (pp[3] & 0x0F) << 1;
	blck[1] = (pp[3] >> 4 & 0x0F) << 1;


	blck[1] |= (blck[8] & 0x02) << 4;
	blck[1] |= (blck[2] & 0x10) >> 4;

	blck[2] |= (blck[1] & 0x02) << 4;
	blck[2] |= (blck[3] & 0x10) >> 4;

	blck[3] |= (blck[2] & 0x02) << 4;
	blck[3] |= (blck[4] & 0x10) >> 4;

	blck[4] |= (blck[3] & 0x02) << 4;
	blck[4] |= (blck[5] & 0x10) >> 4;

	blck[5] |= (blck[4] & 0x02) << 4;
	blck[5] |= (blck[6] & 0x10) >> 4;

	blck[6] |= (blck[5] & 0x02) << 4;
	blck[6] |= (blck[7] & 0x10) >> 4;

	blck[7] |= (blck[6] & 0x02) << 4;
	blck[7] |= (blck[8] & 0x10) >> 4;

	blck[8] |= (blck[7] & 0x02) << 4;
	blck[8] |= (blck[1] & 0x10) >> 4;

	blck[1] ^= (subkey[5] & 0xfc) >> 2;
	blck[2] ^= ((subkey[5] & 0x03) << 4) | ((subkey[4] & 0xf0) >> 4);
	blck[3] ^= ((subkey[4] & 0x0f) << 2) | ((subkey[3] & 0xc0) >> 6);
	blck[4] ^= subkey[3] & 0x3f;
	blck[5] ^= (subkey[2] & 0xfc) >> 2;
	blck[6] ^= ((subkey[2] & 0x03) << 4) | ((subkey[1] & 0xf0) >> 4);
	blck[7] ^= ((subkey[1] & 0x0f) << 2) | ((subkey[0] & 0xc0) >> 6);
	blck[8] ^= subkey[0] & 0x3f;

	blck[1] = S_BOX1[blck[1]];
	blck[2] = S_BOX2[blck[2]];
	blck[3] = S_BOX3[blck[3]];
	blck[4] = S_BOX4[blck[4]];
	blck[5] = S_BOX5[blck[5]];
	blck[6] = S_BOX6[blck[6]];
	blck[7] = S_BOX7[blck[7]];
	blck[8] = S_BOX8[blck[8]];

	//fnc[3] = b[4].0 | b[2].1 | b[5].0 | b[6].3 | b[8].3 | b[3].0 | b[7].0 | b[5].3;
	fnc[3] = (fnc[3] | (blck[4]      & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[2] >> 1 & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[5]      & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[6] >> 3 & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[8] >> 3 & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[3]      & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[7]      & 0x01)) << 1;
	fnc[3] = (fnc[3] | (blck[5] >> 3 & 0x01));

	//fnc[2] = b[1].3 | b[4].1 | b[6].1 | b[7].2 | b[2].3 | b[5].2 | b[8].1 | b[3].2;
	fnc[2] = (fnc[2] | (blck[1] >> 3 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[4] >> 1 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[6] >> 1 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[7] >> 2 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[2] >> 3 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[5] >> 2 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[8] >> 1 & 0x01)) << 1;
	fnc[2] = (fnc[2] | (blck[3] >> 2 & 0x01));

	//fnc[1] = b[1].2 | b[2].0 | b[6].0 | b[4].2 | b[8].0 | b[7].1 | b[1].1 | b[3].3;
	fnc[1] = (fnc[1] | (blck[1] >> 2 & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[2]      & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[6]      & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[4] >> 2 & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[8]      & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[7] >> 1 & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[1] >> 1 & 0x01)) << 1;
	fnc[1] = (fnc[1] | (blck[3] >> 3 & 0x01));

	//fnc[0] = b[5].1 | b[4].3 | b[8].2 | b[2].2 | b[6].2 | b[3].1 | b[1].0 | b[7].3;
	fnc[0] = (fnc[0] | (blck[5] >> 1 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[4] >> 3 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[8] >> 2 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[2] >> 2 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[6] >> 2 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[3] >> 1 & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[1]      & 0x01)) << 1;
	fnc[0] = (fnc[0] | (blck[7] >> 3 & 0x01));
}

void FinalPer(uchar *result)
{
	memset(result, 0, 8);

//	result[0] = pp[3].0 | oo[3].0 | pp[2].0 | oo[2].0 | pp[1].0 | oo[1].0 | pp[0].0 | oo[0].0;
	result[0] = (result[0] | (pp[3] & 0x01)) << 1;
	result[0] = (result[0] | (oo[3] & 0x01)) << 1;
	result[0] = (result[0] | (pp[2] & 0x01)) << 1;
	result[0] = (result[0] | (oo[2] & 0x01)) << 1;
	result[0] = (result[0] | (pp[1] & 0x01)) << 1;
	result[0] = (result[0] | (oo[1] & 0x01)) << 1;
	result[0] = (result[0] | (pp[0] & 0x01)) << 1;
	result[0] = (result[0] | (oo[0] & 0x01));


//	result[1] = pp[3].1 | oo[3].1 | pp[2].1 | oo[2].1 | pp[1].1 | oo[1].1 | pp[0].1 | oo[0].1;
	result[1] = (result[1] | (pp[3] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (oo[3] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (pp[2] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (oo[2] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (pp[1] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (oo[1] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (pp[0] >> 1 & 0x01)) << 1;
	result[1] = (result[1] | (oo[0] >> 1 & 0x01));

//	result[2] = pp[3].2 | oo[3].2 | pp[2].2 | oo[2].2 | pp[1].2 | oo[1].2 | pp[0].2 | oo[0].2;
	result[2] = (result[2] | (pp[3] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (oo[3] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (pp[2] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (oo[2] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (pp[1] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (oo[1] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (pp[0] >> 2 & 0x01)) << 1;
	result[2] = (result[2] | (oo[0] >> 2 & 0x01));

//	result[3] = pp[3].3 | oo[3].3 | pp[2].3 | oo[2].3 | pp[1].3 | oo[1].3 | pp[0].3 | oo[0].3;
	result[3] = (result[3] | (pp[3] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (oo[3] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (pp[2] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (oo[2] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (pp[1] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (oo[1] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (pp[0] >> 3 & 0x01)) << 1;
	result[3] = (result[3] | (oo[0] >> 3 & 0x01));


//	result[4] = pp[3].4 | oo[3].4 | pp[2].4 | oo[2].4 | pp[1].4 | oo[1].4 | pp[0].4 | oo[0].4;
	result[4] = (result[4] | (pp[3] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (oo[3] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (pp[2] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (oo[2] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (pp[1] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (oo[1] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (pp[0] >> 4 & 0x01)) << 1;
	result[4] = (result[4] | (oo[0] >> 4 & 0x01));

//	result[5] = pp[3].5 | oo[3].5 | pp[2].5 | oo[2].5 | pp[1].5 | oo[1].5 | pp[0].5 | oo[0].5;
	result[5] = (result[5] | (pp[3] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (oo[3] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (pp[2] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (oo[2] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (pp[1] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (oo[1] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (pp[0] >> 5 & 0x01)) << 1;
	result[5] = (result[5] | (oo[0] >> 5 & 0x01));

//	result[6] = pp[3].6 | oo[3].6 | pp[2].6 | oo[2].6 | pp[1].6 | oo[1].6 | pp[0].6 | oo[0].6;
	result[6] = (result[6] | (pp[3] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (oo[3] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (pp[2] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (oo[2] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (pp[1] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (oo[1] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (pp[0] >> 6 & 0x01)) << 1;
	result[6] = (result[6] | (oo[0] >> 6 & 0x01));

//	result[7] = pp[3].7 | oo[3].7 | pp[2].7 | oo[2].7 | pp[1].7 | oo[1].7 | pp[0].7 | oo[0].7;
	result[7] = (result[7] | (pp[3] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (oo[3] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (pp[2] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (oo[2] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (pp[1] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (oo[1] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (pp[0] >> 7 & 0x01)) << 1;
	result[7] = (result[7] | (oo[0] >> 7 & 0x01));
}

void des_16(uchar *src, uchar *subkey, uchar *result)
{
	uchar   i,j;
	uchar   fout[4]={0,0,0,0};

    IniPer(src); // output :

	for(i=0;i<15;i++)
	{
		j = i*6;

		desfun(subkey+j); // create fnc[3]..fnc[0]

		fout[3] = oo[3]^fnc[3];
		fout[2] = oo[2]^fnc[2];
		fout[1] = oo[1]^fnc[1];
		fout[0] = oo[0]^fnc[0];

		memcpy(oo, pp, 4);
		memcpy(pp, fout, 4);
	}

	j = i*6;
	desfun(subkey+j);

	oo[3] ^= fnc[3];
	oo[2] ^= fnc[2];
	oo[1] ^= fnc[1];
	oo[0] ^= fnc[0];

	FinalPer(result); // create grslt[0]..grslt[7]
}

// 	Encryption, Decryption 
void Single_DES_Encrypt(uchar *src, uchar *key, uchar *result)  //����
{
	SubKey_Generation(key);       //  enckey[6*16], deckey[6*16]
	des_16(src, enckey, result);  //  encrypted data
}


void Single_DES_Decrypt(uchar *src, uchar *key, uchar *result)  //����
{
	SubKey_Generation(key); 
	des_16(src, deckey, result);  //  decrypted data  
}



 
void  Single_3DES_Encrypt (uchar * src,uchar *key, uchar *result)	 //3DES���ܺ���//
{
	uchar   result1[8],result2[8];

	Single_DES_Encrypt(src, key, result1);        //������Կ��ǰ8���ֽ�
	Single_DES_Decrypt(result1, &key[8], result2);//������Կ�ĺ�8���ֽ�
	Single_DES_Encrypt(result2, key, result);     //����������ܽ����ʹ�õ�����Կ��ǰ8���ֽڵ���Կ	
}
   

void  Single_3DES_Decrypt(uchar * src,uchar *key, uchar *result)	 //3DES���ܺ���//
{
	uchar   result1[8],result2[8];

	Single_DES_Decrypt(src, key, result1);        //������Կ��ǰ8���ֽ�
	Single_DES_Encrypt(result1, &key[8], result2);//������Կ�ĺ�8���ֽ�
	Single_DES_Decrypt(result2, key, result);     //����������ܽ����ʹ�õ�����Կ��ǰ8���ֽڵ���Կ	
}




////////////////////////////////////////////////////////
//�������ܣ�8���ֽڵ������㣬
//���룺������1��Data1
//���룺������2��Data2
//���: �����飻 DataOut
//���أ���
////////////////////////////////////////////////////////
void HexXor(uchar * Data1,uchar *Data2, uchar *DataOut)
{
   uchar i;
   for(i=0;i<8;i++)
   {
   		DataOut[i]=Data1[i]^Data2[i];	
   }
}



////////////////////////////////////////////////////////
//�������ܣ�PBOC_MAC���㣬
//���룺Key���μ��������Կ
//���룺InitialValue : �μ�����ĳ�ʼֵ��һ�������������������8���ֽ�
//����: DataIn:	��Ҫ��������ݿ� ����8���ֽ�Ϊһ��Ϊ��λ��ΪN��
//����: Len: �������ݳ���
//�����DataOut MAC�������8���ֽڵĽ��
//���أ���
////////////////////////////////////////////////////////
void PBOC_MAC(uchar * Key, uchar *InitialValue, uchar *DataIn,uchar Len, uchar *DataOut)
{
	uchar x,y;	
    uchar i,j,k=0;
	uchar  Dbuff[64];
	uchar  buff1[8]={0,0,0,0,0,0,0,0};
	uchar  buff2[8]={0,0,0,0,0,0,0,0};

	memset(Dbuff, 0x00, 64);
    x=Len/8;	//������ж��ٿ�
	y=Len%8;	//�����ʣ�����ֽ�

	for(i=0;i<x;i++)
	{
		for(j=0;j<8;j++)
		{
			Dbuff[k]=DataIn[k];
			k++;
		}
	}

	if(y==0)  //���Y=0��û��ʣ���ֽڣ�����油8���ֽڣ���80Ϊ��ͷ����7���ֽ�Ϊ0;
	{
	    Dbuff[k++]=0x80;
		for(j=0;j<7;j++)
		{
			Dbuff[k++]=0;
		}		
	}
	else	//���Y!=0����ʣ���ֽ� ��ʣ���ֽں��油�빲8���ֽڣ���80Ϊ��ͷ�����ֽ�Ϊ0;
	{

		for(j=0;j<y;j++)
		{
			Dbuff[k]=DataIn[k];
			k++;
		}

		 i=8-y;  //�����ʣ�����ֽ�
		 Dbuff[k++]=0x80;

		 for(j=0;j<i-1;j++)
		 {
			 Dbuff[k++]=0;
		 }
	}



	j=0;
	memcpy(buff2, InitialValue, 4);	 //��ʼֵ����BUFF2Ϊ��һ������׼��
	
	//�ó���Ϊ 8 �ֽڵ���Կ���� MAC ���㷨
	for(i=0;i<x+1;i++)  //x+1 Ϊ�ܵĿ���(����80�ֽں��������)
	{
		HexXor(buff2,&Dbuff[j], buff1); //������
		Single_DES_Encrypt(buff1, Key, buff2);//DES����	  ��Կ����ֽ�
		j+=8;	        //��һ�����ݿ�
	}

	//�ó���Ϊ 16 �ֽڵ���Կ���� MAC ���㷨

	Single_DES_Decrypt(buff2, &Key[8], buff1);//16 �ֽڵ���Կ�ұ��ֽڽ���
	Single_DES_Encrypt(buff1, Key,DataOut );    //16 �ֽڵ���Կ����ֽڼ���

}
