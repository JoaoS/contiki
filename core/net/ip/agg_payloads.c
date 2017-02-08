/**
 * \file
 *      payload aggregator functions
 * \author
 *      André Riker <ariker@dei.uc.pt>
 *		João Subtil <jsubtil@dei.uc.pt>
 *		
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_N_PAYLOADS 40
#define LEN_SINGLE_PAYLOAD 4



struct singlePayload{
	char strContent[LEN_SINGLE_PAYLOAD];
	unsigned int intContent;
};

// DATA STRUCTURE
struct aggPayloads{
	int count_payloads;
	struct singlePayload singleP[MAX_N_PAYLOADS];
	};


static struct aggPayloads PayloadList;




/* The first call to this function is when a packet is produced, so it initializes at 0*/
void reset_payloads(){
	int i;
	for(i=0;i<PayloadList.count_payloads; i=i+1){
		strcpy(PayloadList.singleP[i].strContent,"\n");
		PayloadList.singleP[i].intContent=0;
	}
	PayloadList.count_payloads=0;
	//printf("Parsing: Reseting aggregated payloads \n");
}

/**/
void add_payload(char *content){
			
	strcpy(PayloadList.singleP[PayloadList.count_payloads].strContent,content);
	PayloadList.singleP[PayloadList.count_payloads].intContent=atoi(content);
	
	#if DEBUG_DENSENET
	printf("Parsing: New payload added: int %d string %s num pay %d \n",PayloadList.singleP[PayloadList.count_payloads].intContent,PayloadList.singleP[PayloadList.count_payloads].strContent, PayloadList.count_payloads);
	#endif

	PayloadList.count_payloads=PayloadList.count_payloads+1;
}


unsigned int get_payloads(int payload_position){
	return PayloadList.singleP[payload_position].intContent;
}

char * get_payload_char(int payload_position){
	return PayloadList.singleP[payload_position].strContent;
}

int get_num_payloads(){
	#if DEBUG_DENSENET
	printf("Parsing: Number of agg message %d \n",PayloadList.count_payloads);
	#endif
	return PayloadList.count_payloads;
}







