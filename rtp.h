#include <stdint.h>

#ifndef RTP_H
#define RTP_H

#define RTP_HEADER_SIZE 12
#define RTP_PAYLOAD_MAXSIZE 1400

void rtpHeaderReset(uint8_t *header);
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
					   );

uint8_t rtpHeaderGetV(uint8_t *header);
uint8_t rtpHeaderGetP(uint8_t *header);
uint8_t rtpHeaderGetX(uint8_t *header);
uint8_t rtpHeaderGetCC(uint8_t *header);
uint8_t rtpHeaderGetM(uint8_t *header);
uint8_t rtpHeaderGetPT(uint8_t *header);
uint16_t rtpHeaderGetSEQ(uint8_t *header);
uint32_t rtpHeaderGetTS(uint8_t *header);
uint32_t rtpHeaderGetSSRC(uint8_t *header);

void rtpHeaderSetV(uint8_t v, uint8_t *header);
void rtpHeaderSetP(uint8_t p, uint8_t  *header);
void rtpHeaderSetX(uint8_t x, uint8_t  *header);
void rtpHeaderSetCC(uint8_t cc, uint8_t  *header);
void rtpHeaderSetM(uint8_t m, uint8_t  *header);
void rtpHeaderSetPT(uint8_t pt, uint8_t  *header);
void rtpHeaderSetSEQ(uint16_t seq, uint8_t *header);
void rtpHeaderSetTS(uint32_t ts, uint8_t *header);
void rtpHeaderSetSSRC(uint32_t ssrc, uint8_t *header);

void rtpHeaderShow(uint8_t *header);

#endif // RTP_H
