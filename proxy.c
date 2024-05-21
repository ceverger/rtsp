#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "rtp.h"
#include "h264.h"
#include "rtsp.h"

int main(int argc, char *argv[])
{
	int ret;
	int number = 0;
	uint8_t naluHeader;
	int itSetup = 0, itPlay = 0;

	int clientPort, serverPort;
	int clientRtpPort, clientRtcpPort, serverRtpPort;
	int rtpSocket, clientSocket, connectSocket, serverSocket;
	char serverIP[IPV4_ADDRESS_LEN], clientIP[IPV4_ADDRESS_LEN];
	
	int bufsize = 4096;
	char buffer[bufsize];

	int cacheLen = 2048;
	char cache[cacheLen];

	clientSocket = tcpSocketOpen(true);
	if(clientSocket < 0) return 1;

	ret = tcpSocketConnectToHost(clientSocket, "192.168.5.87", 554);
	if(ret < 0) return 1;

	serverSocket = tcpSocketServer(NULL, 554, 1, true);
	if(serverSocket < 0) return 1;

	connectSocket = tcpSocketConnection(serverSocket, clientIP, &clientPort);
	if(connectSocket < 0) return 1;

	while(1)
	{
		ret = rtspReceiveMessage(connectSocket, buffer, bufsize);
		if(ret < 0) return 1;
		if(ret == 0) break;

		buffer[ret] = '\0';
		printf("%s", buffer);

		if(rtspMethodIsSetup(buffer))
		{
			itSetup = 1;
		}

		if(rtspMethodIsPlay(buffer))
		{
			itPlay = 1;
		}

		ret = rtspSendMessage(clientSocket, buffer, ret);
		if(ret < 0) return 1;		

		ret = rtspReceiveMessage(clientSocket, buffer, bufsize);
		if(ret < 0) return 1;
		if(ret == 0) break;

		buffer[ret] = '\0';
		printf("%s", buffer);
		
		ret = rtspSendMessage(connectSocket, buffer, ret);
		if(ret < 0) return 1;

		if(itSetup)
		{
			rtspParserGetServerPorts(buffer, &clientRtpPort, &clientRtcpPort);			
		}

		if(itPlay)
		{
			rtpSocket = udpSocketServer(0, clientRtpPort, true);
			if(rtpSocket < 0) return 1;
			printf("serverRtpPort = %d\n", clientRtpPort);
			while(1)
			{
				ret = udpSocketReceive(rtpSocket, cache, cacheLen, clientIP, &clientPort);
				if(ret < 0) return 1;

				rtpHeaderShow(cache);

				naluHeader = cache[12];
				printf(
						  "number = %d, forbidden_bit = %d, reference_idc = %d, payload_type = %d, nalu_len = %d, clientIP = %s, clientPort = %d\n\n", 
						   number,
						   h264NaluGetForbiddenBit(naluHeader),
							h264NaluGetReferenceIdc(naluHeader),
						   h264NaluGetPayloadType(naluHeader),
							ret,
							clientIP,
							clientPort
						);

				number++;
			}			
		}
	}

	return 0;
}
