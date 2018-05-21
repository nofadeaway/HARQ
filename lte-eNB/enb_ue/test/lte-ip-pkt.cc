#include "FuncHead.h"

using namespace srslte;
using namespace srsue;

// buffer for reading from tun/tap interface, must be >= 1500 
#define BUFSIZE 2000  
#define dst_ip_start 16

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
	uint8_t ipClient1[4]={192,168,2,1};
	//uint8_t ipClient2[4]={192,168,2,2};
	//uint8_t ipClient3[4]={192,168,2,3};

	while (1) {
 		
		if(k == 1000){
			k = 0;
		}  
		//data from tun/tap: ���ȡsizeof(buffer)������,�ɹ��򷵻ض�ȡ��ʵ���ֽ���nread
		nread = read(tun_fd, buffer[k], BUFSIZE);
 
		if (nread <= 0) {
			perror("Nothing reading");
			exit(1);
		}
  
		sdu_bufs[k].msg     = buffer[k];//index
		sdu_bufs[k].N_bytes = nread; 	//size 

		if((buffer[k][dst_ip_start]==192) && (buffer[k][dst_ip_start+1]==168) && (buffer[k][dst_ip_start+2]==2) && (buffer[k][dst_ip_start+3]==1)){
		
			rlc3.write_sdu(&sdu_bufs[k]);
		}//else if((buffer[k][dst_ip_start]==192) && (buffer[k][dst_ip_start+1]==168) && (buffer[k][dst_ip_start+2]==2) && (buffer[k][dst_ip_start+3]==2))

		sleep(1);  //linux�� \sleep(),���������λ����
		k++;	
	}
}



