#include "TexLib_Private.h"

const int jpegZigzag[64]=
{
	0,	1,	8,	16,	9,	2,	3,	10,
	17,	24,	32,	25,	18,	11,	4,	5,
	12,	19,	26,	33,	40,	48,	41,	34,
	27,	20,	13,	6,	7,	14,	21,	28,
	35,	42,	49,	56,	57,	50,	43,	36,
	29,	22,	15,	23,	30,	37,	44,	51,
	58,	59,	52,	45,	38,	31,	39,	46,
	53,	60,	61,	54,	47,	55,	62,	63
};

typedef unsigned short qTable[64];

typedef struct
{
	struct tables{
		unsigned char size;
		unsigned char code;
	} TSmall[512], TLarge[65536];
} HuffTable;

typedef struct
{
	HuffTable *huffAC, *huffDC;
	qTable *qTab;
	int dcPrev,smpx, smpy;
	float t[256];
}ComponentTable;

struct JpegReader
{
	ComponentTable component[4];
	HuffTable      huffTableAC[4], huffTableDC[4];
	qTable         qtable[4];

	unsigned int xblock, yblock, blockx, blocky,
	bsize, /*restartInt, */bfree,
	dt;
	int restartInt; // TODO: This fixed a warning, but will it still work?
	unsigned char *data, *bpos  ,
	eof  , ssStart, ssEnd,
	sbits, prec   , ncomp;
	float dctt[64];

	unsigned char* dataBuffer;
	int components;
	int width;
	int height;
	TexLib_Context* m_ctx;
	TexLib_Header m_header;

	unsigned char getByte(void)
	{
		unsigned char value;
		if (!m_ctx->m_read(m_ctx, &value, 1))
		{
			eof = 1;
			return 0;
		}
		return value;
	}

	void strmSkip(int n)
	{
		unsigned char a, b;
		bfree+=n;
		dt<<=n;

		while(bfree>=8){
			bfree-=8;
			b = getByte();
			if(b==255)
				a=getByte();
			dt|=(b<<bfree);
		}
	}

	int huffDec(HuffTable *h)
	{
		unsigned int id, n, c;

		id = (dt>>(23));
		n  = h->TSmall[id].size;
		c  = h->TSmall[id].code;

		if(n==255){
			id = (dt>>(16));
			n  = h->TLarge[id].size;
			c  = h->TLarge[id].code;
		}

		strmSkip(n);
		return c;
	}

	int wordDec(int n)
	{
		int w;
		unsigned int s;

		if(n==0)
			return 0;
		else{
			s= (dt>>(31));
			w= (dt>>(32-n));
			strmSkip(n);
			if(s==0)
				w = (w|(0xffffffff<<n))+1;
		}
		return w;
	}

	int getJPGInfo()
	{
		unsigned char cn, sf, qt;
		int i;

		prec = getByte();

		if(prec!=8)
			return 0;

		height = ((int) getByte() << 8) + getByte();
		width  = ((int) getByte() << 8) + getByte();
		ncomp  = getByte();

		if((ncomp!=3)&&(ncomp!=1))
			return 0;

		m_header.m_width = width;
		m_header.m_height = height;
		m_header.m_depth = 0;
		m_header.m_hasColor = true;
		m_header.m_hasAlpha = false;
		m_header.m_format = TexLib_Format_RGB;
		m_header.m_numLevels = 1;
		m_header.m_numFaces = 1;
		if (!m_ctx->m_onHeader(m_ctx, &m_header))
			return 0;

		for(i=0;i<ncomp;i++)
		{
			cn = getByte();
			sf = getByte();
			qt = getByte();

			component[cn-1].qTab = &qtable[qt];
			component[cn-1].smpy = sf&15;
			component[cn-1].smpx = (sf>>4)&15;
		}

		if(component[0].smpx == 1)
			blockx = 8;
		else
			blockx = 16;

		if(component[0].smpy==1)
			blocky = 8;
		else
			blocky = 16;

		xblock=width/blockx;

		if((width & (blockx-1))!=0)
			xblock++;

		yblock = height/blocky;

		if((height&(blocky-1))!=0)
			yblock++;
		return 1;
	}

	void decodeHuffTable(int len)
	{
		int length[257], i, j, n, code, codelen, delta, rcode, cd, rng;
		unsigned char lengths[16], b, symbol[257];
		HuffTable *h;

		len-=2;

		while(len>0){
			b = getByte();
			len--;
			h = &huffTableDC[0];

			switch(b){
		  case 0:
			  h = &huffTableDC[0];
			  break;

		  case 1:
			  h = &huffTableDC[1];
			  break;

		  case 16:
			  h = &huffTableAC[0];
			  break;

		  case 17:
			  h=&huffTableAC[1];
			  break;
			}

			for(i=0;i<16;i++)
				lengths[i] = getByte();

			len -= 16;
			n    =  0;

			for(i=0;i<16;i++){
				len-=lengths[i];
				for(j=0;j<lengths[i];j++){
					symbol[n]   = getByte();
					length[n++] = i+1;
				}
			}

			code = 0;
			codelen = length[0];

			for(i=0;i<n;i++){

				rcode = code<<(16-codelen);
				cd = rcode>>7;

				if(codelen<=9){
					rng = 1 <<(9-codelen);

					for(j=cd;j<cd+rng;j++){
						h->TSmall[j].code = (unsigned char)symbol[i];
						h->TSmall[j].size = (unsigned char)codelen;
					}
				}
				else{
					h->TSmall[cd].size=(unsigned char)255;
					rng = 1<<(16-codelen);

					for(j=rcode;j<rcode+rng;j++){
						h->TLarge[j].code=(unsigned char)symbol[i];
						h->TLarge[j].size=(unsigned char)codelen;
					}
				}

				code++;
				delta=length[i+1]-codelen;
				code<<=delta;
				codelen+=delta;
			}
		}
	}

	void fidct(void)
	{
		float a = 0.353553385f,
			b = 0.490392625f,
			c = 0.415734798f,
			d = 0.277785122f,
			e = 0.097545162f,
			f = 0.461939752f,
			g = 0.191341713f,
			cd =0.6935199499f,
			be =0.5879377723f,
			bc =0.9061274529f,
			de =0.3753302693f,
			a0, f2, g2, a4, f6, g6, s0, s1, s2, s3,
			t0, t1, t2, t3, m0, m1, m2, m3,
			h0, h1, h2, h3, r0, r1, r2, r3, w;
		int i;

		for(i=0;i<64;i+=8){
			if((dctt[i+1]!=0)||(dctt[i+2]!=0)||(dctt[i+3]!=0)||(dctt[i+4]!=0)||(dctt[i+5]!=0)||(dctt[i+6]!=0)||(dctt[i+7]!=0))
			{
				a0 = a*dctt[i];
				f2 = f*dctt[i+2];
				g2 = g*dctt[i+2];
				a4 = a*dctt[i+4];
				g6 = g*dctt[i+6];
				f6 = f*dctt[i+6];
				m0 = a0+a4;
				m1 = a0-a4;
				m2 = f2+g6;
				m3 = g2-f6;
				s0 = m0+m2;
				s1 = m1+m3;
				s2 = m1-m3;
				s3 = m0-m2;
				h2 = dctt[i+7]+dctt[i+1];
				h3 = dctt[i+7]-dctt[i+1];
				r2 = dctt[i+3]+dctt[i+5];
				r3 = dctt[i+3]-dctt[i+5];
				h0 = cd*dctt[i+1];
				h1 = be*dctt[i+1];
				r0 = be*dctt[i+5];
				r1 = cd*dctt[i+3];
				w = de*r3;
				t0 = h1+r1+e*(h3+r3)-w;
				t1 = h0-r0-d*(h2-r3)-w;
				w = bc*r2;
				t2 = h0+r0+c*(h3+r2)-w;
				t3 = h1-r1-b*(h2+r2)+w;
				dctt[i] = s0+t0;
				dctt[i+1] = s1+t1;
				dctt[i+2] = s2+t2;
				dctt[i+3] = s3+t3;
				dctt[i+4] = s3-t3;
				dctt[i+5] = s2-t2;
				dctt[i+6] = s1-t1;
				dctt[i+7] = s0-t0;
			}
			else{
				a0 = dctt[i]*a;
				dctt[i]=dctt[i+1]=dctt[i+2]=dctt[i+3]=dctt[i+4]=dctt[i+5]=dctt[i+6]=dctt[i+7]=a0;
			}
		}

		for(i=0;i<8;i++){
			if((dctt[8+i]!=0)||(dctt[16+i]!=0)||(dctt[24+i]!=0)||(dctt[32+i]!=0)||(dctt[40+i]!=0)||(dctt[48+i]!=0)||(dctt[56+i]!=0))
			{
				a0 = a*dctt[i];
				f2 = f*dctt[16+i];
				g2 = g*dctt[16+i];
				a4 = a*dctt[32+i];
				g6 = g*dctt[48+i];
				f6 = f*dctt[48+i];
				m0 = a0+a4;
				m1 = a0-a4;
				m2 = f2+g6;
				m3 = g2-f6;
				s0 = m0+m2;
				s1 = m1+m3;
				s2 = m1-m3;
				s3 = m0-m2;
				h2 = dctt[56+i]+dctt[8+i];
				h3 = dctt[56+i]-dctt[8+i];
				r2 = dctt[24+i]+dctt[40+i];
				r3 = dctt[24+i]-dctt[40+i];
				h0 = cd*dctt[8+i];
				h1 = be*dctt[8+i];
				r0 = be*dctt[40+i];
				r1 = cd*dctt[24+i];
				w = de*r3;
				t0 = h1+r1+e*(h3+r3)-w;
				t1 = h0-r0-d*(h2-r3)-w;
				w = bc*r2;
				t2 = h0+r0+c*(h3+r2)-w;
				t3 = h1-r1-b*(h2+r2)+w;
				dctt[i] = s0+t0;
				dctt[i+8] = s1+t1;
				dctt[i+16] = s2+t2;
				dctt[i+24] = s3+t3;
				dctt[i+32] = s3-t3;
				dctt[i+40] = s2-t2;
				dctt[i+48] = s1-t1;
				dctt[i+56] = s0-t0;
			}
			else{
				a0 = dctt[i]*a;
				dctt[i]=dctt[i+8]=dctt[i+16]=dctt[i+24]=dctt[i+32]=dctt[i+40]=dctt[i+48]=dctt[i+56]=a0;
			}
		}
	}

	void decodeQTable(int len)
	{
		int i;
		unsigned char b;

		len-=2;

		while(len>0){
			b = (unsigned char)getByte();
			len--;

			if((b&16)==0){
				for(i=0;i<64;i++)
					qtable[b&15][i]=getByte();
				len-=64;
			}
			else{
				for(i=0;i<64;i++)
					qtable[b&15][i]=((getByte()<<8)+getByte());
				len-=128;
			}
		}
	}

	void decodeBlock(void)
	{
		int compn, i, j, b, p, codelen, code, cx, cy, otab[64];
		qTable *qtab;

		for(compn=0;compn<ncomp;compn++)
		{
			qtab = component[compn].qTab;

			for(cy=0;cy<component[compn].smpy;cy++){
				for(cx=0;cx<component[compn].smpx;cx++){
					for(i=0;i<64;i++)
						otab[i]=0;

					codelen= huffDec(component[compn].huffDC);
					code=wordDec(codelen);

					otab[0] = code+component[compn].dcPrev;
					component[compn].dcPrev = otab[0];
					i=1;

					while(i<64){
						codelen=huffDec(component[compn].huffAC);
						if(codelen==0)
							i=64;
						else
							if(codelen==0xf0)
								i+=16;
							else{
								code = wordDec(codelen&15);
								i = i+(codelen>>4);
								otab[i++]=code;
							}
					}

					for(i=0;i<64;i++)
						dctt[jpegZigzag[i]]=(float)((*qtab)[i]*otab[i]);

					fidct();
					b=(cy<<7)+(cx<<3);
					p=0;

					for(i=0;i<8;i++){
						for(j=0;j<8;j++)
							component[compn].t[b++]=dctt[p++]+128;
						b+=8;
					}
				}
			}
		}
	}

	int decodeScanJPG()
	{
		unsigned int nnx, nny, i, j,
			xmin, ymin, xmax, ymax, blockn, adr1, adr2,
			y1, u1, v1, y2, u2, v2, u3, v3,
			dux, duy, dvx, dvy;
		unsigned char sc, ts;
		float cy, cu, cv;
		components = (int) getByte();

		dataBuffer = (unsigned char*) m_ctx->m_alloc(m_ctx, width * height * components);
		if (!dataBuffer)
			return 0;

		for(i=0;i<components;i++){
			sc = getByte();
			ts = getByte();

			component[sc-1].huffDC = &huffTableDC[ts>>4];
			component[sc-1].huffAC = &huffTableAC[ts&15];
		}

		ssStart = getByte();
		ssEnd   = getByte();
		sbits   = getByte();

		if((ssStart!=0)||(ssEnd!=63))
			return 0;

		if(components == 3){
			dux = 2+component[1].smpx-component[0].smpx;
			duy = 2+component[1].smpy-component[0].smpy;
			dvx = 2+component[2].smpx-component[0].smpx;
			dvy = 2+component[2].smpy-component[0].smpy;
		}

		dt    = 0;
		bfree = 0;
		strmSkip(32);

		blockn=0;
		ymin=0;

		for(nny=0;nny<yblock;nny++){
			ymax = ymin+blocky;

			if(ymax>height)
				ymax = height;

			xmin=0;

			for(nnx=0;nnx<xblock;nnx++){
				xmax=xmin+blockx;

				if(xmax>width)
					xmax=width;

				decodeBlock();

				blockn++;
				if((blockn==(unsigned int)restartInt)&&((nnx<xblock-1)||(nny<yblock-1)))
				{
					blockn=0;

					if(bfree!=0)
						strmSkip(8-bfree);

					if(wordDec(8)!=255)
						return 0;

					for(i=0;i<components;i++)
						component[i].dcPrev=0;
				}

				if(components ==3){
					y1=u1=v1=0;
					adr1=(height-1-ymin)*width+xmin;

					for(i=ymin;i<ymax;i++){
						adr2=adr1;
						adr1-=width;
						y2=y1;
						y1+=16;
						u3=(u1>>1)<<4;
						u1+=duy;
						v3=(v1>>1)<<4;
						v1+=dvy;
						u2=v2=0;

						for(j=xmin;j<xmax;j++){
							int cr, cg, cb;
							cy=component[0].t[y2++];
							cu=component[1].t[u3+(u2>>1)]-128.0f;
							cv=component[2].t[v3+(v2>>1)]-128.0f;

							cr=(int)(cy+1.402f*cv);
							cg=(int)(cy-0.34414f*cu-0.71414f*cv);
							cb=(int)(cy+1.772f*cu);

							dataBuffer[3*adr2]   = max(0, min(255, cb));
							dataBuffer[3*adr2+1] = max(0, min(255, cg));
							dataBuffer[3*adr2+2] = max(0, min(255, cr));
							adr2++;

							u2+=dux;
							v2+=dvx;
						}
					}
				}
				else
					if(components==1)
					{
						y1=0;
						adr1=(height-1-ymin)*width+xmin;

						for(i=ymin;i<ymax;i++){
							adr2=adr1;
							adr1-=width;
							y2=y1;
							y1+=16;

							for(j=xmin;j<xmax;j++){
								int lum=(int)component[0].t[y2++];
								dataBuffer[adr2++]=max(0, min(255, lum));
							}
						}
					}
					xmin=xmax;
			}
			ymin=ymax;
		}
		return 1;
	}

	int decodeJPG()
	{
		int w;
		unsigned char a, hdr=0, scan=0;

		eof=0;
		w=((getByte()<<8)+getByte());

		if(w!=0xffd8)
			return 0;

		while(eof==0)
		{
			a=(unsigned char)getByte();

			if(a!=0xff)
				return 0;

			a=(unsigned char)getByte();
			w=((getByte()<<8)+getByte());

			switch(a){
		  case 0xe0:
			  if(hdr!=0)
				  break;

			  if(getByte()!='J')
				  return 0;

			  if(getByte()!='F')
				  return 0;

			  if(getByte()!='I')
				  return 0;

			  if(getByte()!='F')
				  return 0;

			  hdr=1;
			  w-=4;
			  break;

		  case 0xc0:
			  if (!getJPGInfo())
				  return 0;
			  w=0;
			  break;

		  case 0xc4:
			  decodeHuffTable(w);
			  w=0;
			  break;

		  case 0xd9:
			  w=0;
			  break;

		  case 0xda:
			  if(scan!=0)
				  break;

			  scan=1;

			  if(!decodeScanJPG())
				  return 0;

			  w=0;
			  eof=1;
			  break;

		  case 0xdb:
			  decodeQTable(w);
			  w=0;
			  break;

		  case 0xdd:
			  restartInt=((getByte()<<8)+getByte());
			  w=0;
			  break;
			}

			while(w>2){
				getByte();
				w--;
			}
		}
		return 1;
	}

};

bool TexLib_LoadJPG(TexLib_Context* ctx)
{
	JpegReader* reader = (JpegReader*) ctx->m_alloc(ctx, sizeof(JpegReader));
	if (!reader)
		return false;
	for (int i=0;i<4;i++)
		reader->component[i].dcPrev=0;
	reader->restartInt=-1;
	reader->dataBuffer = NULL;
	reader->m_ctx = ctx;

	if (!reader->decodeJPG())
	{
		if (reader->dataBuffer)
			ctx->m_free(ctx, reader->dataBuffer);
		ctx->m_free(ctx, reader);
		return false;
	}

	TexLib_Data_RAW rawData;
	rawData.m_curr = reader->dataBuffer;
	rawData.m_end = reader->dataBuffer + reader->width * reader->height * reader->components;

	const bool result = TexLib_TransformRawImage(ctx, &reader->m_header, &rawData);
	ctx->m_free(ctx, reader->dataBuffer);
	ctx->m_free(ctx, reader);

	return result;
}