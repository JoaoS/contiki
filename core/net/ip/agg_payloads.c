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
#define LEN_SINGLE_PAYLOAD 2


struct singlePayload{
	char strContent[LEN_SINGLE_PAYLOAD];
	unsigned int intContent;
};

// DATA STRUCTURE
struct aggPayloads{
	int count_payloads;
	struct singlePayload singleP[MAX_N_PAYLOADS];
	};


struct aggPayloads aggP;

// FUNCTIONS
void reset_payloads(){
	int i;
	for(i=0;i<aggP.count_payloads; i=i+1){
		strcpy(aggP.singleP[i].strContent,"\n");
		aggP.singleP[i].intContent=0;
	}
	aggP.count_payloads=0;
	//printf("Parsing: Reseting aggregated payloads \n");
}

/**/
void add_payload(char *content){
	char temp1[2];
	
	strcpy(aggP.singleP[aggP.count_payloads].strContent,content);
		
	temp1[0] = (int) aggP.singleP[aggP.count_payloads].strContent[0];
	temp1[1] = (int) aggP.singleP[aggP.count_payloads].strContent[1];
	
	aggP.singleP[aggP.count_payloads].intContent=atoi(temp1);
	printf("Parsing: New payload added: int %d string %s num pay %d \n",aggP.singleP[aggP.count_payloads].intContent,aggP.singleP[aggP.count_payloads].strContent, aggP.count_payloads);
	aggP.count_payloads=aggP.count_payloads+1;
}


int get_payloads(int payload_position){

return aggP.singleP[payload_position].intContent;

}
char * get_payload_char(int payload_position){
	return aggP.singleP[payload_position].strContent;
}

int get_num_payloads(){
//printf("Parsing: Number of agg message %d \n",aggP.count_payloads);
return aggP.count_payloads;
}







