#include "FuncHead.h"

using namespace srslte;
using namespace srsue;

// buffer for reading from tun/tap interface, must be >= 1500 
#define BUFSIZE 2000  

extern rlc_um rlc3;
extern int tun_fd;

/**************************************************************************
* lte_send_ip_3:��tun�ж����ݲ�ѹ�����
**************************************************************************/

void* lte_send_ip_3(void *ptr) {

	printf("enter--lte_send_ip_3\n");

	/******************************************
	* tun_alloc
	******************************************/
	uint16_t nread;
	uint8_t buffer[1000][BUFSIZE] ={0};
 
	/*****************************************
	* read from tun and write to rlc
	******************************************/	
	byte_buffer_t sdu_bufs[1000];//��ʱ��Ϊ1000�����ڶ�д��Ȼ�����ص�
 
	int k = 0;

	while (1) {
 		
		if(k == 1000){
			k = 0;
		}  
		//data from tun/tap: ���ȡsizeof(buffer)������,�ɹ��򷵻ض�ȡ��ʵ���ֽ���nread
		nread = read(tun_fd, buffer[k], BUFSIZE);//size�޸���--�������,���ڶ���
 		
		if (nread <= 0) {
			perror("Nothing reading");
			exit(1);
		}
  
		sdu_bufs[k].msg = buffer[k]; //index,���ַ
		sdu_bufs[k].N_bytes = nread; //size 
		
		rlc3.write_sdu(&sdu_bufs[k]);
  
		usleep(10);  //linux�� sleep(m),���������λ����
		k++;	
	}
}



