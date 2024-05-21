#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "network.h"
#include "rtp.h"

int load_file(const char *filename, uint8_t **buf, int *bufsize);

uint8_t *aggregation_nalu(
									uint8_t *pos, 
									uint8_t *end, 
									uint8_t *cache, 
									uint8_t *cache_end, 
									uint8_t *nri,
									int *len
								 );

uint8_t *aggregation_packet(
									 uint8_t *pos, 
									 uint8_t *end, 
									 uint8_t *cache, 
									 uint8_t *cache_end, 
									 int *len
								   );

uint8_t *single_packet(
							  uint8_t *pos, 
							  uint8_t *end, 
							  uint8_t *cache, 
							  int *len
						    );

void rtpHeaderReset(uint8_t *header)
{
	
}

void rtpHeaderInit(
						  uint8_t v, 
						  uint8_t p, 
						  uint8_t x, 
						  uint8_t cc, 
						  uint8_t m, 
						  uint8_t pt, 
						  uint16_t seq, 
						  uint32_t ts, 
						  uint32_t ssrc,
					     uint8_t *header
					   )
{
	header[0] = ((v << 6) & 0xC0) | (header[0] & 0x3F);
	header[0] = ((p << 5) & 0x20) | (header[0] & 0xDF);
	header[0] = ((x << 4) & 0x10) | (header[0] & 0xEF);
	header[0] = (cc & 0x0F) | (header[0] & 0xF0);
	
	header[1] = ((m << 7) & 0x80) | (header[1] & 0x7F);
	header[1] = (pt & 0x7F) | (header[1] & 0x80);

	header[2] = (seq >> 8) & 0x00FF;
	header[3] = seq & 0x00FF;

	header[4] = (ts >> 24) & 0x000000FF;
	header[5] = (ts >> 16) & 0x000000FF;
	header[6] = (ts >> 8)  & 0x000000FF;
	header[7] = ts & 0x000000FF;

	header[8]  = (ssrc >> 24) & 0x000000FF;
	header[9]  = (ssrc >> 16) & 0x000000FF;
	header[10] = (ssrc >> 8)  & 0x000000FF;
	header[11] =  ssrc & 0x000000FF;

}

uint8_t rtpHeaderGetV(uint8_t *header)
{
	uint8_t v = (header[0] >> 6) & 0x03;
	
	return v;
}

uint8_t rtpHeaderGetP(uint8_t *header)
{
	uint8_t p = (header[0] >> 5) & 0x01;

	return p;
}

uint8_t rtpHeaderGetX(uint8_t *header)
{
	uint8_t x = (header[0] >> 4) & 0x01;

	return x;
}

uint8_t rtpHeaderGetCC(uint8_t *header)
{
	uint8_t cc = header[0] & 0x0F;

	return cc;	
}

uint8_t rtpHeaderGetM(uint8_t *header)
{
	uint8_t m = (header[1] >> 7)  & 0x01;

	return m;	
}

uint8_t rtpHeaderGetPT(uint8_t *header)
{
	uint8_t pt = header[1] & 0x7F;

	return pt;
}

uint16_t rtpHeaderGetSEQ(uint8_t *header)
{
	uint16_t seq = header[2];

	seq = (seq << 8) | header[3];

	return seq;

}

uint32_t rtpHeaderGetTS(uint8_t *header)
{
	uint32_t ts = 0;
	uint32_t temp = header[4];

	ts = (temp << 24);

	temp = header[5];

	ts |= temp << 16;

	temp = header[6];

	ts |= temp << 8;

	temp = header[7];

	ts |= temp;

	return ts;
}

uint32_t rtpHeaderGetSSRC(uint8_t *header)
{
	uint32_t ssrc = 0;
	uint32_t temp = header[8];

	ssrc = (temp << 24);

	temp = header[9];

	ssrc |= temp << 16;

	temp = header[10];

	ssrc |= temp << 8;

	temp = header[11];

	ssrc |= temp;

	return ssrc;	
}

void rtpHeaderSetV(uint8_t v, uint8_t *header)
{
	header[0] = ((v << 6) & 0xC0) | (header[0] & 0x3F);	
}

void rtpHeaderSetP(uint8_t p, uint8_t*header)
{
	header[0] = ((p << 5) & 0x20) | (header[0] & 0xDF);	
}

void rtpHeaderSetX(uint8_t x, uint8_t *header)
{
	header[0] = ((x << 4) & 0x10) | (header[0] & 0xEF);	
}

void rtpHeaderSetCC(uint8_t cc, uint8_t *header)
{
	header[0] = (cc & 0x0F) | (header[0] & 0xF0);	
}

void rtpHeaderSetM(uint8_t m, uint8_t *header)
{
	header[1] = ((m << 7) & 0x80) | (header[1] & 0x7F);	
}

void rtpHeaderSetPT(uint8_t pt, uint8_t *header)
{
	header[1] = (pt & 0x7F) | (header[1] & 0x80);	
}

void rtpHeaderSetSEQ(uint16_t seq, uint8_t *header)
{
	header[2] = (seq >> 8) & 0x00FF;
	header[3] = seq & 0x00FF;	
}

void rtpHeaderSetTS(uint32_t ts, uint8_t *header)
{
	header[4] = (ts >> 24) & 0x000000FF;
	header[5] = (ts >> 16) & 0x000000FF;
	header[6] = (ts >> 8)  & 0x000000FF;
	header[7] = ts & 0x000000FF;	
}

void rtpHeaderSetSSRC(uint32_t ssrc, uint8_t *header)
{
	header[8]  = (ssrc >> 24) & 0x000000FF;
	header[9]  = (ssrc >> 16) & 0x000000FF;
	header[10] = (ssrc >> 8)  & 0x000000FF;
	header[11] =  ssrc & 0x000000FF;	
}

void rtpHeaderShow(uint8_t *header)
{
	printf(
	        "RTP Header: v = %d, p = %d, x = %d, cc = %d, m = %d, pt = %d, seq = %d, ts = %d, ssrc = %d\n",
			   rtpHeaderGetV(header),
				rtpHeaderGetP(header),
				rtpHeaderGetX(header),
				rtpHeaderGetCC(header),
				rtpHeaderGetM(header),
				rtpHeaderGetPT(header),
				rtpHeaderGetSEQ(header),
				rtpHeaderGetTS(header),
				rtpHeaderGetSSRC(header)
	      );
}
