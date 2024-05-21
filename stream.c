#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "network.h"
#include "h264.h"
#include "rtp.h"

int h264RtpStream(
						const char *filename, 
						long ssrc, 
						int rtpSocket,
						const char *clientIP, 
						int clientRtpPort
					  )
{
	if(filename == NULL || clientIP == NULL)
	{
		printf("invalid arguments\n");
		return -1;
	}

	int port;
	char ipaddr[IPV4_ADDRESS_LEN];

	int ret; 
	int count, bufsize;
	int packetNumber, otherSize;
	uint8_t *buffer, *pos, *end, *payload;

	uint32_t payloadLen;
	uint8_t referenceIdc, payloadType;
	uint8_t naluHeader, fuaIndicator, fuaHeader;

	int cacheLen = 12;
	int cacheSize = 1412;
	uint8_t cache[cacheSize];
	uint8_t *cachePos = cache + cacheLen;
	uint8_t *cacheEnd = cache + cacheSize;

	socketInfo(rtpSocket, ipaddr, &port);
	printf("ip = %s, port = %d\n\n", ipaddr, port);

	ret = h264LoadFile(filename, &buffer, &bufsize);
	if(ret < 0) return 1;

	pos = buffer;
	end = buffer + bufsize;

	count = h264NaluCount(pos, end);
	printf("NALU count = %d\n", count);

	rtpHeaderInit(2, 0, 0, 0, 0, 96, 0, 0, ssrc, cache);

	while(pos < end)
	{		
		naluHeader = h264NaluGetHeader(pos);
		payload = h264NaluPayload(pos);
		payloadLen = h264NaluGetPayloadLen(pos, end);
		
		if(payloadLen < 1400)
		{
			pos = h264AggregationPacket(pos, end, cache, cachePos, cacheEnd, &cacheLen);

			if(cacheLen > payloadLen + 15)
			{
				ret = udpSocketSend(rtpSocket, cache, cacheLen, clientIP, clientRtpPort);
				if(ret < cacheLen) return 1;
				cacheLen = 12;
				rtpHeaderSetSEQ(rtpHeaderGetSEQ(cache) + 1, cache);
				if(h264NaluGetPayloadType(h264NaluGetHeader(pos)) == 9)
				{
					rtpHeaderSetTS(rtpHeaderGetTS(cache) + 90000 / 60, cache);
					usleep(1000 * 1000 / 60);
				}
			}
			else
			{
				pos = h264SinglePacket(pos, end, cachePos, &cacheLen);

				ret = udpSocketSend(rtpSocket, cache, cacheLen, clientIP, clientRtpPort);
				if(ret < cacheLen) return 1;

				cacheLen = 12;
				rtpHeaderSetSEQ(rtpHeaderGetSEQ(cache) + 1, cache);
				rtpHeaderSetTS(rtpHeaderGetTS(cache) + 90000 / 60, cache);
				usleep(1000 * 1000 / 60);	
			}
		}
		else
		{
			packetNumber = (payloadLen - 1) / 1398;
			otherSize = (payloadLen - 1) % 1398;

			fuaIndicator = h264NaluGetFuaIndicator(naluHeader);
			fuaHeader = h264NaluGetFuaHeader(naluHeader);
			payload++;

			for(int i = 0; i < packetNumber; ++i)
			{
				if(i == 0) 
					fuaHeader = h264NaluSetFuaStartBit(1, fuaHeader);
				if(i == 1)
					fuaHeader = h264NaluResetFuaBits(fuaHeader);

				*cachePos++ = fuaIndicator;
				*cachePos++ = fuaHeader;
				memcpy(cachePos, payload, 1398);
				cachePos += 1398;
				payload += 1398;
				
				ret = udpSocketSend(rtpSocket, cache, cacheSize, clientIP, clientRtpPort);
				if(ret < 0) return 1;

				cacheLen = 12;
				cachePos = cache + cacheLen;
				rtpHeaderSetSEQ(rtpHeaderGetSEQ(cache) + 1, cache);
				fuaHeader = h264NaluResetFuaBits(fuaHeader);
			}

			if(otherSize > 0)
			{
				fuaHeader = h264NaluSetFuaEndBit(1, fuaHeader);

				*cachePos++ = fuaIndicator;
				*cachePos++ = fuaHeader;
				memcpy(cachePos, payload, otherSize);
				cachePos += otherSize;
				payload += otherSize;

				ret = udpSocketSend(rtpSocket, cache, cachePos - cache, clientIP, clientRtpPort);
				if(ret < 0) return 1;

				cacheLen = 12;
				cachePos = cache + cacheLen;
				rtpHeaderSetSEQ(rtpHeaderGetSEQ(cache) + 1, cache);
				rtpHeaderSetTS(rtpHeaderGetTS(cache) + 90000 / 60, cache);
				pos = h264NaluFind(pos, end);
				usleep(1000 * 1000 / 60);
			}
		}
	}

	return 0;
}
