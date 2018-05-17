#include "FuncHead.h"

#define PAYLAODSIZE 400

using namespace srslte;
using namespace srsue;
 
extern demux mac_demux_test;
extern mac_dummy_timers timers_test;
/**************************************************************************
* ipsend:从tun中读数据并压入队列
**************************************************************************/

void* lte_rece(void *ptr) {

	printf("enter--lte_rece\n");

	int st = socket(AF_INET, SOCK_DGRAM, 0);
	if (st == -1) {
		printf("open socket failed ! error message : %s\n", strerror(errno));
		exit(1);//return -1;
	}
	int port = atoi("4404");
	 
	struct sockaddr_in addr;
	 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	 
	if (bind(st, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		printf("bind IP failed ! error message : %s\n", strerror(errno));
		exit(1);
	}
 
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
 
	int rece_size = 300, k = 0;;//修改为随机啊！！！！！！！！！！！ 
	uint8_t rece_payload[1000][PAYLAODSIZE] ={0};

	while (1) {

		if(k == 1000){
			k = 0;
		} 
		 
		memset(&client_addr, 0, sizeof(client_addr));

		if (recvfrom(st, rece_payload[k], rece_size, 0, (struct sockaddr *)&client_addr, &addrlen) == -1) {

			printf("recvfrom failed ! error message : %s\n", strerror(errno));
			goto END;
		}
		else {
			//MAC->RLC->IP 第二个参数有误,先固定与接收端一致,但是貌似不影响解包,丢弃了
			mac_demux_test.process_pdu(rece_payload[k], rece_size);
			 
			while(!timers_test.get(-1)->is_expired()){ timers_test.get(-1)->step();}	
		}
		k++;	
	}
 
	END:close(st); 
}
