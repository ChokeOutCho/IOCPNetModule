#include <iostream>
#include "NetLib/NetLib_Server.h"
#include "Utils/Parser.h"
#include "conio.h"
#include "Utils/SystemMonitor.h"

class ExampleServer :public NetLib_Server
{
public:
	ExampleServer(const WCHAR* openIP,
		unsigned short openPort,
		int opt_workerTH_Pool_size,
		int opt_concurrentTH_size,
		unsigned short opt_maxOfSession,
		bool opt_zerocpy,
		Opt_Encryption* opt_encryption,
		int opt_maxOfSendPackets)
		:NetLib_Server(openIP,
			openPort,
			opt_workerTH_Pool_size,
			opt_concurrentTH_size,
			opt_maxOfSession,
			opt_zerocpy,
			opt_encryption,
			opt_maxOfSendPackets)
	{

	}

	virtual void OnClientJoin(unsigned long long sessionHandle)
	{

		return;
	}

	virtual void OnClientLeave(unsigned long long sessionHandle)
	{
		return;
	}

	virtual bool OnConnectionRequest(unsigned long IP, unsigned short port)
	{
		// 다 받음
		return true;
	}

	virtual void OnRecv(unsigned long long sessionHandle, Packet* packet)
	{
		PROFILING("CONTENT");

		WORD contentType;
		*packet >> contentType;
		Content_Proc(sessionHandle, contentType, packet);
	}

	void Content_Proc(unsigned long long sessionHandle, WORD contentType, Packet* packet)
	{
		switch (contentType)
		{
		case 0:
		{
			long long echoData;
			*packet >> echoData;
			Packet* echoPacket = Packet::Alloc();
			*echoPacket << (WORD)contentType;
			*echoPacket << echoData;
			SendPacket(sessionHandle, packet);
			Packet::Free(echoPacket);
			break;
		}
		default:
		{
			break;
		}
		}
	}

	SystemMonitor system_monitor;
};

ExampleServer* exampleserver;

int main()
{
	int opt_serverport;
	int opt_poolsize;
	int opt_concurrentsize;
	int opt_maxofsession;
	bool opt_zerocpy;
	char opt_encryption_header_code;
	char opt_encryption_fixed_key;
	int opt_sendbuf;
	Parser parser;
	if (parser.loadFromFile("server_config.cfg"))
	{
		opt_serverport = parser.GetInt("port");
		opt_poolsize = parser.GetInt("workerTH_Pool_size");
		opt_concurrentsize = parser.GetInt("concurrentTH_size");
		opt_maxofsession = parser.GetInt("maxofsession");

		opt_encryption_header_code = (char)parser.GetInt("encryption_header_code");
		opt_encryption_fixed_key = (char)parser.GetInt("encryption_fixed_key");

		opt_zerocpy = parser.GetBool("zerocopy");
		opt_sendbuf = parser.GetInt("sendbuf");
	}
	else
	{
		printf("config파일이 없습니다!!!");
		wint_t key = _getwch();
		return -1;
	}

	Opt_Encryption opt_encryption = { opt_encryption_header_code, opt_encryption_fixed_key };
	exampleserver = new ExampleServer(L"0.0.0.0", opt_serverport, opt_poolsize, opt_concurrentsize,
		opt_maxofsession, opt_zerocpy, &opt_encryption, opt_sendbuf);

	bool isRunning = true;

	exampleserver->Start();
	printf("Server Start\n");

	while (isRunning)
	{
		printf("Q누르면 종료\n");
		printf("\n\nSession Count:              %7d", exampleserver->GetSessionCount());
		printf("\nPacket Pool Use:            %7d", exampleserver->GetPacketUseSize());
		printf("\nTotal_Accept:               %7d", exampleserver->GetTotal_Accept());
		printf("\nTPS_Accept:                 %7d", exampleserver->GetTPS_Accept());
		printf("\nTPS_Send:                   %7d", exampleserver->GetTPS_Send());
		printf("\nTPS_Recv:                   %7d", exampleserver->GetTPS_Recv());
		printf("\n-------------------------------------------------------------------------\n");

		if (_kbhit())
		{
			wint_t key = _getwch();

			if (key == 'q' || key == 'Q')
			{
				isRunning = false;
			}


		}
		Sleep(1000);
	}
}