#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "h264.h"

static bool startcode3(uint8_t *buf)
{
	/* Поиск в видео-файле кодека h264 стартового кода длиной 3 байта */
	
	return (buf[0] == 0 && buf[1] == 0 && buf[2] == 1) ? true : false;
}

static bool startcode4(uint8_t *buf)
{
	/* Поиск в видео-файле кодека h264 стартового кода длиной 4 байта */

	return (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1) ? true : false;
}

int h264NaluCount(uint8_t *pos, uint8_t *end)
{
	/* Подсчёт количества nal_unit в видеофайле кодека h264 */
	
	int count = 0;

	while(pos < end)
	{
		if(startcode3(pos) || startcode4(pos))
		{
			while(*pos++ != 1);
			count++;
			continue;
		}

		pos++;
	}
	
	return count;
}

uint8_t *h264NaluFind(uint8_t *pos, uint8_t *end)
{
	if(startcode3(pos) || startcode4(pos))
		while(*pos++ != 1);

	while(pos < end)
	{
		if(startcode3(pos) || startcode4(pos)) return pos;
		pos++;
	}
	
	return end;
}

uint8_t *h264NaluPayload(uint8_t *pos)
{
	if(startcode3(pos) || startcode4(pos))
		while(*pos++ != 1);
	else
		pos = NULL;

	return pos;	
}


int h264NaluGetPayloadLen(uint8_t *pos, uint8_t *end)
{
	if(startcode3(pos) || startcode4(pos))
		while(*pos++ != 1);

	int len = 0;
	uint8_t *next;

	next = h264NaluFind(pos, end);

	len = next - pos;

	return len;
}

uint8_t h264NaluGetHeader(uint8_t *pos)
{
	if(startcode3(pos) || startcode4(pos))
		while(*pos++ != 1);

	return *pos;
}

uint8_t h264NaluGetForbiddenBit(uint8_t header)
{	
	return header >> 7;	
}

uint8_t h264NaluGetReferenceIdc(uint8_t header)
{
	return (header >> 5) & 0x03;
}

uint8_t h264NaluGetPayloadType(uint8_t header)
{
	return header & 0x1F;
}

uint8_t h264NaluSetForbiddenBit(uint8_t forbiddenBit, uint8_t header)
{
	header = (forbiddenBit << 7) | (header & 0x7F);

	return header;
}

uint8_t h264NaluSetReferenceIdc(uint8_t referenceIdc, uint8_t header)
{
	header = ((referenceIdc << 5) & 0x60) | (header & 0x9F);

	return header;
}

uint8_t h264NaluSetPayloadType(uint8_t payloadType, uint8_t header)
{
	header = (payloadType & 0x1F) | (header & 0xE0);

	return header;
}

uint8_t h264NaluGetStapaHeader(uint8_t header)
{
	header = (header & 0xE0) | 24;

	return header;
}

uint8_t h264NaluGetFuaIndicator(uint8_t header)
{
	header = (header & 0xE0) | 28;

	return header;
}

uint8_t h264NaluGetFuaHeader(uint8_t header)
{
	header = header & 0x1F;

	return header;
}

uint8_t h264NaluGetFuaStartBit(uint8_t fuaHeader)
{
	uint8_t startBit = (fuaHeader & 0x80) >> 7;

	return startBit;
}

uint8_t h264NaluGetFuaEndBit(uint8_t fuaHeader)
{
	uint8_t endBit = (fuaHeader & 0x40) >> 6;

	return endBit;
}

uint8_t h264NaluSetFuaStartBit(uint8_t startBit, uint8_t fuaHeader)
{
	fuaHeader = (fuaHeader & 0x1F) | (startBit << 7);
	
	return fuaHeader;
}

uint8_t h264NaluSetFuaEndBit(uint8_t endtBit, uint8_t fuaHeader)
{
	fuaHeader = (fuaHeader & 0x1F) | (endtBit << 6);
}

uint8_t h264NaluResetFuaBits(uint8_t fuaHeader)
{
	return fuaHeader & 0x1F;
}

uint8_t *h264SinglePacket(
								  uint8_t *pos, 
								  uint8_t *end, 
								  uint8_t *cachePos,
								  int *cacheLen
						       )
{
	if(pos == end) return pos;

	uint8_t *payload    = h264NaluPayload(pos);
	uint32_t payloadLen = h264NaluGetPayloadLen(pos, end);

	memcpy(cachePos, payload, payloadLen);
	
	*cacheLen += payloadLen;

	pos = h264NaluFind(pos, end);

	return pos;
}

uint8_t *aggregationNalu(
									uint8_t *pos,
									uint8_t *end,
									uint8_t *cachePos,
									uint8_t *cacheEnd,
									int *cacheLen,
									uint8_t *nri
								)
{
	if(pos == end) return pos;
	if(pos == end) return pos;
	if(pos == NULL || end == NULL) return pos;
	if(cachePos == NULL || cacheEnd == NULL || cacheLen == NULL) return pos;

	int ret;
	uint8_t stapaHeader;

	uint8_t  naluHeader   = h264NaluGetHeader(pos);
	uint32_t payloadLen   = h264NaluGetPayloadLen(pos, end);
	uint8_t  *payload     = h264NaluPayload(pos);
	uint8_t  referenceIdc = h264NaluGetReferenceIdc(naluHeader);	
	uint8_t  payloadType  = h264NaluGetPayloadType(naluHeader);

	if(payloadType != 9)
	{
		if(payloadLen + 2 < cacheEnd - cachePos)
		{
			*nri = *nri < referenceIdc ? referenceIdc : *nri;
			*cachePos++ = (uint8_t)((payloadLen >> 8) & 0x000000FF);
			*cachePos++ = (uint8_t)(payloadLen & 0x000000FF);
			memcpy(cachePos, payload, payloadLen);
			cachePos += payloadLen;
			*cacheLen  += payloadLen + 2;

			pos = h264NaluFind(pos, end);
			pos = aggregationNalu(pos, end, cachePos, cacheEnd, cacheLen, nri);
		}
	}

	return pos;
}

uint8_t *h264AggregationPacket(
									    uint8_t *pos, 
									    uint8_t *end, 
									    uint8_t *cache, 
										 uint8_t *cachePos, 
									    uint8_t *cacheEnd,
									    int *cacheLen
                              )
{
	if(pos == end) return pos;
	if(pos == NULL || end == NULL) return pos;
	if(cache == NULL || cachePos == NULL || cacheEnd == NULL || cacheLen == NULL) return pos;

	int ret;
	uint8_t stapaHeader;

	uint8_t  naluHeader   = h264NaluGetHeader(pos);
	uint32_t payloadLen   = h264NaluGetPayloadLen(pos, end);
	uint8_t  *payload     = h264NaluPayload(pos);
	uint8_t  referenceIdc = h264NaluGetReferenceIdc(naluHeader);	
	uint8_t  payloadType  = h264NaluGetPayloadType(naluHeader);

	stapaHeader  = h264NaluGetStapaHeader(naluHeader);

	if(payloadType == 9)
	{
		stapaHeader = h264NaluGetStapaHeader(naluHeader);
		*cachePos++ = stapaHeader;
		*cachePos++ = (uint8_t)((payloadLen >> 8) & 0x000000FF);
		*cachePos++ = (uint8_t)(payloadLen & 0x000000FF);
		memcpy(cachePos, payload, payloadLen);
		cachePos  += payloadLen;
		*cacheLen += payloadLen + 3;

		pos = h264NaluFind(pos, end);
		pos = aggregationNalu(pos, end, cachePos, cacheEnd, cacheLen, &referenceIdc);
		cache[12] = h264NaluSetReferenceIdc(referenceIdc, stapaHeader);
	}

	return pos;
}


int h264LoadFile(const char *filename, uint8_t **buf, int *bufsize)
{
	/* Загрузка потока видео-файла h264 в буфер */
	
	if(filename == NULL || buf == NULL)
	{
		printf("h264LoadFile(): invalid argument");
		return -1;
	}
	
	int fd;
	struct stat file_info;

	fd = open(filename, O_RDONLY);
	
	if(fd < 0)
	{
		perror("open()");
		return -1;
	}
	
	if(fstat(fd, &file_info) < 0)
	{
		perror("fstat()");
		return 1;
	}
	
	*bufsize = (int) file_info.st_size;	
	*buf = (uint8_t *) calloc(*bufsize, sizeof(uint8_t));

	if(*buf == NULL)
	{
		printf("h264LoadFile(): allocation error");
		return -1;
	}
	
	if(read(fd, *buf, *bufsize) < 0)
	{
		perror("read");
		return -1;
	}

	return 0;	
}
