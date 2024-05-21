#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rtsp.h"

#define request "OPTIONS rtsp://192.168.5.114:8554/test.264 RTSP/1.0\r\nCSeq: 2\r\nUser-Agent: LibVLC/3.0.20 (LIVE555 Streaming Media v2016.11.28)\r\n\r\n"

int main(int argc, char *argv[])
{
	int ret, cseq, count;

	int methodLen = 16;
	char method[methodLen];

	int uriLen = 256;
	char uri[uriLen];

	int nameLen = 256;
	char name[nameLen];

	rtspParserGetMethod(request, method, methodLen);
	printf("%s\n", method);

	rtspParserGetURI(request, uri, uriLen);
	printf("%s\n", uri);

	rtspParserGetFilename(uri, name, nameLen);
	printf("%s\n", name);

	cseq = rtspParserGetCSeq(request);
	printf("%d\n", cseq);

	return 0;

}
