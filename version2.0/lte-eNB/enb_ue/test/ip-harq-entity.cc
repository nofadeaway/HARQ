#include "FuncHead.h"

#define SEND_SIZE 400

using namespace srslte;
using namespace srsue; 

extern mux ue_mux_test;
extern srslte::pdu_queue pdu_queue_test;   //5.28
bool ACK[8]={false,false,false,false,false,false,false,false};


uint8_t* trans_control(uint32_t pid_now) {

    uint8_t *payload_test = new uint8_t[SEND_SIZE];
	uint8_t *payload_back = new uint8_t[SEND_SIZE];

	uint32_t pdu_sz_test = 300;//下面其实应该发送最终打包长度吧，待修改
	uint32_t tx_tti_test = 1;
	uint32_t pid_test = 8; //目前暂时只有1个进程
	//uint32_t pid_now = 0;
	int retrans_limit=3;
    int retrans_times[8]={0};
	//begin{5.29}
	uint8_t* payload_tosend = new uint8_t[SEND_SIZE];
    bool qbuff_flag=false;   //记录 qbuff::send()返回值
	//end{5.29}


    while(1){
    
	memset(payload_test, 0, SEND_SIZE*sizeof(uint8_t));
    memset(payload_back, 0, SEND_SIZE*sizeof(uint8_t));

	payload_back = ue_mux_test.pdu_get(payload_test, pdu_sz_test, tx_tti_test, pid_now);
	printf("Now this pdu belongs to HARQ NO.%d\n",pid_now);
		
		
	//begin{5.28添加}
        if(pdu_queue_test.request_buffer(pid_now,pdu_sz_test))     //request_buffer函数返回指为qbuff中ptr指针，而在下面send中其实并不需要使用
		{printf("PID No.%d:queue's buffer request succeeded!\n",pid_now);}
		
		qbuff_flag=pdu_queue_test.pdu_q[pid_now].send(payload_back,pdu_sz_test);     //把payload)back存入qbuff
		if(qbuff_flag){
			printf("Succeed in sending PDU to queue's buffer!\n");
		}
		else{
			printf("Fail in sending PDU to queue's buffer!\n");
		}
    

	/***********************************************
	    *控制重发
	    *************************************************/
        
        if(ACK[pid_now])
		{
           //payload_tosend = payload_back;
		   printf("Now PID No.%d:queue's No.%d buffer will be sent.\n",pid_now,pdu_queue_test.pdu_q[pid_now].wp_is()); //接收到ACK，发送下一个PDU
		   pdu_queue_test.pdu_q[pid_now].release();                                                         //将前一个已经收到对应ACK的PDU丢弃
		   return payload_back;
		}    
		else        //没收到ACK，重发
		{
		   //memcpy(payload_tosend, pdu_queue_test.pdu_q[pid_now].pop(pdu_sz_test,1), pdu_sz_test);
           retrans_times[pid_now]++;   
           if(retrans_times[pid_now]<retrans_limit){
		   uint32_t len=pdu_sz_test;
           //payload_tosend =(uint8_t*) pdu_queue_test.pdu_q[pid_now].pop(&len);   //暂时是前7个进程一直ACK为true，第8个ACK一直为false
		   printf("Now PID NO.%d:the retransmission size is %d bytes.\n",pid_now,len);
		   printf("Now PID No.%d:queue's No.%d buffer will be sent.\n",pid_now,pdu_queue_test.pdu_q[pid_now].rp_is()+1);
		   
		   return  (uint8_t*) pdu_queue_test.pdu_q[pid_now].pop(&len);
		   }
		   else{
			   retrans_times[pid_now]=0;
			   pdu_queue_test.pdu_q[pid_now].release();                             //丢弃超过重发次数的PDU
		   }
		}  
		
       /*******************************************/
    }
}
