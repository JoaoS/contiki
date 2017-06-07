/**
 * \file
 *      payload aggregator functions
 * \author
 *      André Riker <ariker@dei.uc.pt>
 *		João Subtil <jsubtil@dei.uc.pt>
 *		
 */


#include "contiki.h"
#include "agg_payloads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static aggPayloads PayloadList = {0};

//The first call to this function is when a packet is produced, so it initializes at 0
void reset_payloads(void){
	memset(&PayloadList, 0, sizeof(aggPayloads));
	
	#if DEBUG_DENSENET
	printf("Parsing: Reseting aggregated payloads \n");
	#endif
}

singlePayload * ask_list(int index){
	return &PayloadList.singleP[index];
}

int payload_number(void){
	return PayloadList.number_payloads;
}
/********************************************/

/*specific funtions to handle the agg header*/
int parse_fix_agg_header(uint8_t * payload){
	int flag=0;
	uint8_t ext_concat = (FIX_AGG_HEADER_ALLOW_CONCAT_MASK & *payload) >> FIX_AGG_HEADER_ALLOW_CONCAT_POSITION;
	uint8_t	nr_payloads = (FIX_AGG_HEADER_NRPAYLOADS_MASK & * payload) >>FIX_AGG_HEADER_NRPAYLOADS_POSITION;

	/*i can always put my payload in, if a payload unit has my groupID, else i have to check if i can concatenate*/
	flag=check_for_my_group(payload, nr_payloads);
	if(flag){
		//parse and save packets for later
		save_to_buffer(payload,nr_payloads);
		//discard remaining headers
		return 1;
	}
	/*check if i can concatenate, if not forward*/
	else{
		if (EXTERNAL_CONCAT)
		{	
			/*can i add my payload to the packet*/
			if (ext_concat)
			{
				/* if i have more than 1 group in the packet always concatenate externally */
				save_to_buffer(payload,nr_payloads);
				return 1;
			}
			else
				return 0;
			
		}else{
			//forward my packet
			printf("forward this packet \n");
			return 0;

		}
	}
}

/*return 1 if i have my group in there, return zero otherwise*/
int check_for_my_group(uint8_t * payload, int nr_payloads){

	int i=1; /*skip header*/
	uint8_t var, groupid;
	//go through every packet, 
	for (; i <= nr_payloads*4; i=i+4)
	{
		var= payload[i];
		groupid = (var & GROUPID_MASK) >> GROUPID_POSITION;
		if(groupid==GROUPID)
		return 1;
	}
	return 0;	
}

void save_to_buffer(uint8_t * payload, int nr_payloads){

	int i=1; //skip header
	uint8_t var, groupid, agg_function, total_num;
	uint16_t payload_val;

	//go through every packet and add it to the buffer according to the agg function
	for (; i <= nr_payloads*4; i=i+4)
	{	

		var=groupid=agg_function=total_num=0;
		payload_val=0;

		var= payload[i];
		groupid = (var & GROUPID_MASK) >> GROUPID_POSITION;
		agg_function = (var & AGG_FUNCTION_MASK)>>AGG_FUNCTION_POSITION;
		var= payload[i+1];
		total_num=(var & NRVALUES_MASK) >> NRVALUES_POSITION;
		//the order of payload is inverted, the endianess is different
		var= payload[i+2];
		payload_val = (var & PAYLOAD_MASK);
		var = payload[i+3];
		payload_val |= (var & PAYLOAD_MASK) << 8;
		printf("group id =%u\n",groupid);


		/*insert the data in the buffer, check first if it has data inside*/
		if(PayloadList.singleP[groupid].flag){

			PayloadList.singleP[groupid].total_num = PayloadList.singleP[groupid].total_num+total_num;
			
			if(agg_function==0){minPayload(PayloadList.singleP[groupid].value , payload_val);}
			else if(agg_function==1){avgPayload(PayloadList.singleP[groupid].value , payload_val);printf("resultado=%u\n",avgPayload(PayloadList.singleP[groupid].value , payload_val) );}
			else if(agg_function==2){maxPayload(PayloadList.singleP[groupid].value , payload_val);}
		}
		else{	
			PayloadList.singleP[groupid].flag = 1;
			PayloadList.singleP[groupid].groupID = groupid;
			PayloadList.singleP[groupid].agg_function = agg_function;
			PayloadList.singleP[groupid].total_num = total_num;
			PayloadList.singleP[groupid].value = payload_val;
			PayloadList.singleP[groupid].reserved = 0;
			PayloadList.number_payloads=PayloadList.number_payloads+1;
		}
	}
}
/********************************************/

/*in group operations*/
uint16_t minPayload(uint16_t saved ,uint16_t current){
	return saved < current ? saved : current;
}

uint16_t avgPayload(uint16_t saved ,uint16_t current){
	return ((saved+current)/2);
}

uint16_t maxPayload(uint16_t saved ,uint16_t current){
	return saved > current ? saved : current;
}
/********************************************/
