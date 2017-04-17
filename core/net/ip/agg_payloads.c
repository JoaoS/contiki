/**
 * \file
 *      payload aggregator functions
 * \author
 *      André Riker <ariker@dei.uc.pt>
 *		João Subtil <jsubtil@dei.uc.pt>
 *		
 */


#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*data from received payload*/
struct singlePayload{
	unsigned int payload;

};

/*array of payloads to send*/
struct aggPayloads{
	int count_payloads;	/*total number of received*/
	struct singlePayload singleP[MAX_N_PAYLOADS];	/*space for each packet*/
};

static struct aggPayloads PayloadList={0};



/* The first call to this function is when a packet is produced, so it initializes at 0*/
void reset_payloads(){
	int i;
	for(i=0;i<PayloadList.count_payloads; i=i+1){
		PayloadList.singleP[i].payload=0;
	}
	PayloadList.count_payloads=0;
	
	#if DEBUG_DENSENET
		printf("Parsing: Reseting aggregated payloads \n");
	#endif
}

/**/
void add_payload(uint8_t *incomingPayload){
	uint8_t temp[LEN_SINGLE_PAYLOAD];
	memcpy(temp,incomingPayload,LEN_SINGLE_PAYLOAD);

	#if DEBUG_DENSENET
		printf("incoming=%s temp=%s\n",incomingPayload,temp );
	#endif
	
	PayloadList.singleP[PayloadList.count_payloads].payload=atoi((char*)temp);
	PayloadList.count_payloads=PayloadList.count_payloads+1;
}

unsigned int get_payloadAt(int payload_position){
	return PayloadList.singleP[payload_position].payload;
}


int get_num_payloads(){
	return PayloadList.count_payloads;
}