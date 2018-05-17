#include "FuncHead.h"

#define SEND_SIZE 400 

using namespace srslte;
using namespace srsue; 
  
extern mux ue_mux_test;

void* lte_send_udp(void *ptr) {

	printf("enter--lte_send_udp\n");

	int port = atoi("5505");
	//create socket
	int st = socket(AF_INET, SOCK_DGRAM, 0);
	if (st == -1)
	{
		printf("create socket failed ! error message :%s\n", strerror(errno));
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("192.168.3.254");//目的实际地址
 
	uint8_t *payload_test = new uint8_t[SEND_SIZE];
	uint8_t *payload_back = new uint8_t[SEND_SIZE];

	uint32_t pdu_sz_test = 300;//下面其实应该发送最终打包长度吧，待修改
	uint32_t tx_tti_test = 1;
	uint32_t pid_test = 1;//啥用啊

	while (1) {

		memset(payload_test, 0, SEND_SIZE*sizeof(uint8_t));
		memset(payload_back, 0, SEND_SIZE*sizeof(uint8_t));

	     	//uint8_t* mux::pdu_get(uint8_t *payload, uint32_t pdu_sz, uint32_t tx_tti, uint32_t pid)
		
		payload_back = ue_mux_test.pdu_get(payload_test, pdu_sz_test, tx_tti_test, pid_test);
 
		if (sendto(st, payload_back, pdu_sz_test, 0, (struct sockaddr *) &addr,
			sizeof(addr)) == -1)
		{
			printf("sendto failed ! error message :%s\n", strerror(errno));
			break;
		} 
		sleep(1);//usleep(10);
	}

	delete[] payload_test;
	delete[] payload_back;

	close(st);
}
