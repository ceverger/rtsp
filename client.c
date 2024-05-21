#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "h264.h"
#include "rtp.h"
#include "rtsp.h"

void itob(uint8_t value, uint8_t *buf, int bufsize);

int main(int argc, char *argv[])
{

	if(argc < 2)
	{
		printf("invalid_arguments\n");
		return 1;
	}

	int ret;
	int sockfd;
	int cseq = 1;
	int number = 0;
	uint8_t naluHeader;

	int bufsize = 4096;
	char buffer[bufsize];

	int rtpPort = 9832;
	int rtcpPort = 9833;
	int rtpSocket, rtcpSocket;

	int port;
	char ipaddr[IPV4_ADDRESS_LEN];

	int sessionSize = 512;
	char session[sessionSize];
	
	sockfd = tcpSocketOpen(true);
	ret = tcpSocketConnectToHost(sockfd, "192.168.5.114", 8554);
	if(ret < 0) return 1;

	rtpSocket = udpSocketServer(0, rtpPort, true);
	if(rtpSocket < 0) return 1;

	rtcpSocket = udpSocketServer(0, rtcpPort, true);
	if(rtcpSocket < 0) return 1;

	ret = rtspRequestForOption(cseq, argv[1], "Rutronica Client", buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);
	cseq++;

	ret = tcpSocketSend(sockfd, buffer, ret);
	if(ret <= 0) return 1;

	ret = tcpSocketReceive(sockfd, buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	ret = rtspRequestForDescribe(cseq, argv[1], "Rutronica Client", "application/sdp", buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);
	cseq++;

	ret = tcpSocketSend(sockfd, buffer, ret);
	if(ret <= 0) return 1;

	ret = tcpSocketReceive(sockfd, buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	ret = rtspRequestForSetup(cseq, argv[1], "Rutronica Client", "RTP/AVP", "unicast", rtpPort, rtcpPort, buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	ret = tcpSocketSend(sockfd, buffer, ret);
	if(ret <= 0) return 1;

	ret = tcpSocketReceive(sockfd, buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	rtspParserGetSession(buffer, session, sessionSize);
	printf("Session = %s\n", session);

	ret = rtspRequestForPlay(cseq, argv[1], "Rutronica Client", session, "npt=0.000-", buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	ret = tcpSocketSend(sockfd, buffer, ret);
	if(ret <= 0) return 1;

	ret = tcpSocketReceive(sockfd, buffer, bufsize);
	if(ret <= 0) return 1;
	printf("%s\n", buffer);

	while(1)
	{
		ret = udpSocketReceive(rtpSocket, buffer, bufsize, ipaddr, &port);
		if(ret < 0) return 1;

		rtpHeaderShow(buffer);

		naluHeader = buffer[12];
		printf(
				  "number = %d, forbidden_bit = %d, reference_idc = %d, payload_type = %d, nalu_len = %d, clientIP = %s, clientPort = %d\n\n", 
				   number,
				   h264NaluGetForbiddenBit(naluHeader),
					h264NaluGetReferenceIdc(naluHeader),
				   h264NaluGetPayloadType(naluHeader),
					ret - 12,
					ipaddr,
					port
				);

		number++;
	}	
		
	return 0;
}

void itob(uint8_t value, uint8_t *buf, int bufsize)
{
	uint8_t n = 0;
	
	for(int i = 0; i < bufsize; ++i)
	{
		n = (value >> bufsize - (i + 1)) & 0x01;
		buf[i] = n;
	}
}
