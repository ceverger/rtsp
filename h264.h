#include <stdint.h>

#ifndef H264_H
#define H264_H

	int h264NaluCount(uint8_t *pos, uint8_t *end);
	uint8_t *h264NaluFind(uint8_t *pos, uint8_t *end);
	uint8_t *h264NaluPayload(uint8_t *pos);

	int h264NaluGetPayloadLen(uint8_t *pos, uint8_t *end);
	uint8_t h264NaluGetHeader(uint8_t *pos);
	uint8_t h264NaluGetForbiddenBit(uint8_t header);
	uint8_t h264NaluGetReferenceIdc(uint8_t header);
	uint8_t h264NaluGetPayloadType(uint8_t header);
	
	uint8_t h264NaluSetForbiddenBit(uint8_t forbiddenBit, uint8_t header);
	uint8_t h264NaluSetReferenceIdc(uint8_t referenceIdc, uint8_t header);
	uint8_t h264NaluSetPayloadType(uint8_t payloadType, uint8_t header);

	uint8_t h264NaluGetStapaHeader(uint8_t header);

	uint8_t h264NaluGetFuaIndicator(uint8_t header);
	uint8_t h264NaluGetFuaHeader(uint8_t header);
	uint8_t h264NaluGetFuaStartBit(uint8_t fuaHeader);
	uint8_t h264NaluGetFuaEndBit(uint8_t fuaHeader);
	uint8_t h264NaluSetFuaStartBit(uint8_t startBit, uint8_t fuaHeader);
	uint8_t h264NaluSetFuaEndBit(uint8_t endtBit, uint8_t fuaHeader);
	uint8_t h264NaluResetFuaBits(uint8_t fuaHeader);


	uint8_t *aggregationNalu(
									 uint8_t *pos,
									 uint8_t *end,
									 uint8_t *cachePos,
									 uint8_t *cacheEnd,
									 int *cacheLen,
									 uint8_t *nri
								   );

	uint8_t *h264AggregationPacket(
										    uint8_t *pos,
										    uint8_t *end, 
										    uint8_t *cache,
											 uint8_t *cachePos,
										    uint8_t *cacheEnd,
										    int *cacheLen
								   	   );

	uint8_t *h264SinglePacket(
								 	  uint8_t *pos, 
								     uint8_t *end, 
								     uint8_t *cachePos,
								     int *cacheLen
						       	 );

	int h264CreateSDP(
							const char *serverIP, 
							int serverRtpPort, 
							int framerate, 
							char *buf, 
							int bufsize
						  );

	int h264LoadFile(
						  const char *filename, 
                    uint8_t **buf, 
                    int *bufsize
                   );

#endif // H264_H
