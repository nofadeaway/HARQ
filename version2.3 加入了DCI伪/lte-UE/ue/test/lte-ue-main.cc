#include "FuncHead.h"

using namespace srslte;
using namespace srsue;
 
pthread_t id[3];//3个线程

rlc_um rlc3; 
mac_dummy_timers timers_test; 
mux ue_mux_test;
demux mac_demux_test;
demux mac_demux_test_trans;      //用于发送方的，其中自动会有pdu_queue
srslte::pdu_queue pdu_queue_test; //自己添加的PDU排队缓存,目前支持的HARQ进程数最多为8，既最多缓存8个PDU
 
int tun_fd;// option;全局变量--rlc写入ip时用

/**************************************************************************
* rlc-class
**************************************************************************/


class rlc_um_tester_3
	:public pdcp_interface_rlc
	, public rrc_interface_rlc
{
public:
	rlc_um_tester_3() { n_sdus = 0; }

	// PDCP interface
	void write_pdu(uint32_t lcid, byte_buffer_t *sdu)
	{
		assert(lcid == 3);
		sdus[n_sdus++] = sdu;
	}

	// RRC interface
	void max_retx_attempted() {}

	byte_buffer_t *sdus[5];
	int n_sdus;
};


/////////////////
class rlc_mac_tester
	:public rlc_interface_mac  
{
public:
  	 uint32_t get_buffer_state(uint32_t lcid)
	{if(lcid==3){
		return rlc3.get_buffer_state();
	}
	//else{//lcid==4以及其他
	//	return rlc4.get_buffer_state();
	//}	
	}

   uint32_t get_total_buffer_state(uint32_t lcid) {}

   const static int MAX_PDU_SEGMENTS = 20;

  /* MAC calls RLC to get RLC segment of nof_bytes length.
   * Segmentation happens in this function. RLC PDU is stored in payload. */
   	int read_pdu(uint32_t lcid, uint8_t *payload, uint32_t nof_bytes)
	{
		int len;
		if(lcid==3){
			len=rlc3.read_pdu(payload,nof_bytes);
		}
		//else{//lcid==4以及其他
			//len=rlc4.read_pdu(payload,nof_bytes);printf("HERE4444\n");
		//}
		return len;
	}

	/* MAC calls RLC to push an RLC PDU. This function is called from an independent MAC thread.
	* PDU gets placed into the buffer and higher layer thread gets notified. */
	void write_pdu(uint32_t lcid, uint8_t *payload, uint32_t nof_bytes)
	{
		if (lcid == 3) {

			rlc3.write_pdu(payload, nof_bytes); 
		}//else{//lcid==4以及其他
		 //rlc4.write_pdu(payload,nof_bytes);printf("HERE4444\n");
		 //}	
	}
};

/**************************************************************************
* thread_create----thread_wait
**************************************************************************/
void thread_create(void){

	int temp;
	memset(&id, 0, sizeof(id));

	if ((temp = pthread_create(&id[0], NULL, lte_send_ip_3, NULL) != 0))
		//参数：线程标识符指针 线程属性  线程运行函数起始地址  运行函数属性;创建成功返回 0
		printf("Thread 1 lte_send_ip_3 fail to create!\n");
	else
		printf("Thread 1 lte_send_ip_3 created\n");
	 
	//FX 7.2 修改
	// if( (temp = pthread_create(&id[1], NULL, lte_send_udp, NULL) != 0))
	// 	printf("Thread 2 lte_send_udp fail to create!\n");
	// else
	// 	printf("Thread 2 lte_send_udp created!\n");

	if( (temp = pthread_create(&id[2], NULL, lte_rece, NULL) != 0))
		printf("Thread 3 lte_rece fail to create!\n");
	else
		printf("Thread 3 lte_rece created!\n");
}


void thread_wait()
{
	if (id[0] != 0)
	{
		pthread_join(id[0], NULL); //等待线程结束，使用此函数对创建的线程资源回收
		printf("Thread1 lte_send_ip_3 completed!\n");
	}
	if (id[1] != 0)
	{
		pthread_join(id[1], NULL);
		printf("Thread2 lte_send_udp completed!\n");
	}
	if (id[2] != 0)
	{
		pthread_join(id[2], NULL);
		printf("Thread3 lte_rece completed!\n");
	}
}


/**************************************************************************
* main
**************************************************************************/
int main(void)
{ 	
	/******************************************
	* tun_alloc
	******************************************/
	char tun_name[IFNAMSIZ];
	int  flags = IFF_TUN;//或者IFF_TAP

	// Connect to the device 
	strcpy(tun_name, "tun1");
	tun_fd = tun_alloc(tun_name, flags | IFF_NO_PI); // tun interface 
	if (tun_fd < 0) {
		perror("Allocating interface");
		exit(1);
	}

	/**********************************************
	* rlc-um-1-lcid-3
	***********************************************/
	srslte::log_stdout log3("RLC_UM_3");
	
	log3.set_level(srslte::LOG_LEVEL_DEBUG);
	
	log3.set_hex_limit(-1);
	
	rlc_um_tester_3    tester_3;
	//mac_dummy_timers timers; 
 
	rlc3.init(&log3, 3, &tester_3, &tester_3, &timers_test);//LCID=3!!!!!!

	LIBLTE_RRC_RLC_CONFIG_STRUCT cnfg;
	cnfg.rlc_mode = LIBLTE_RRC_RLC_MODE_UM_BI;
	cnfg.dl_um_bi_rlc.t_reordering = LIBLTE_RRC_T_REORDERING_MS5;
	cnfg.dl_um_bi_rlc.sn_field_len = LIBLTE_RRC_SN_FIELD_LENGTH_SIZE10;
	cnfg.ul_um_bi_rlc.sn_field_len = LIBLTE_RRC_SN_FIELD_LENGTH_SIZE10;

	rlc3.configure(&cnfg);

	/***********************************************
	* MAC-MUX
	*************************************************/
	srslte::log_stdout log2("UE_MUX");
	log2.set_level(srslte::LOG_LEVEL_DEBUG);
	log2.set_hex_limit(-1); 

	rlc_mac_tester rlc_test;
	bsr_proc bsr_test;
	phr_proc phr_test;

	log2.set_level(srslte::LOG_LEVEL_DEBUG);//

	ue_mux_test.init(&rlc_test, &log2, &bsr_test, &phr_test);


	/***********************************************
	* MAC-PDU-Queue 将MAC的PDU存入缓冲队列    控制重发先写在lte-udp.cc里
	*************************************************/
    srslte::log_stdout log4("EnB_Queue");
	log4.set_level(srslte::LOG_LEVEL_DEBUG);
	log4.set_hex_limit(-1);

	log4.set_level(srslte::LOG_LEVEL_DEBUG);//

	pdu_queue::process_callback*  callback_test; //
	callback_test = &mac_demux_test_trans; // 5.23


	pdu_queue_test.init(callback_test,&log4);            //存入PDU的操作写在lte-udp.cc里面
/***********************************************
	* ACK发送与接受，目前基站端接受，UE发送
	*************************************************/

	/***********************************************
	* MAC-DEMUX
	*************************************************/
	srslte::log_stdout log1("UE_DEMUX");
	log1.set_level(srslte::LOG_LEVEL_DEBUG);
	log1.set_hex_limit(-1);  

	phy_interface_mac phy_interface_mac_test;//物理层与mac接口--/ue/hdr/phy_interface
	//srslte::timers timers_test;//这是啥---屏蔽掉了,在MAC CE处理出有用
	log1.set_level(srslte::LOG_LEVEL_DEBUG);

	mac_demux_test.init(&phy_interface_mac_test, &rlc_test, &log1);//,&timers_test);
 
	printf("Main fuction,creating thread...\n");
	thread_create();
	printf("Main fuction, waiting for the pthread end!\n");
	thread_wait();
	return (0);
}


