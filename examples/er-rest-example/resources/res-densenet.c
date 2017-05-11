/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */
#include "contiki.h"
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "lib/random.h"
#include "net/ip/agg_payloads.h"
#include <node-id.h>
#include <math.h>
#include "core/sys/energest.h"



/*subtil*/
#include "sys/clock.h"

#define MAX_INT 9999
#define RES_DEBUG 1



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);
static int trans_count=1000;
void payloadConcat(char * test);
void maxPayload(char * test, unsigned int current);
void minPayload(char * test,unsigned int current);
void avgPayload(char * test,unsigned int current);

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
PERIODIC_RESOURCE(res_densenet,
         "title=\"Aggregation: ?len=0..\";rt=\"Text\";obs",
         res_get_handler,
         NULL,
         NULL,
         NULL,
         20*CLOCK_SECOND,
         res_periodic_handler);
/*20 seconds =255*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	/*
	*	This is where we set the packet payload and annex other payloads if available
	*	first we should check if packets are available for processing, 
	*
	*/
	char test[MAX_N_PAYLOADS];
	int i=0;
	for(i=0;i<MAX_N_PAYLOADS;i=i+1){test[i]='\0';}

	/*Each message has node id + tansmission count**/
	test[0]=NODE_ID+'0';
	sprintf(test+1,"%d",0);
	
	/*
	test[5]=NODE_ID+'0';
	sprintf(test+6,"%d",trans_count);
	
	test[10]=NODE_ID+'0';
	sprintf(test+11,"%d",trans_count);
	/*
	test[15]=NODE_ID+'0';
	sprintf(test+16,"%d",trans_count);	
	2
	test[20]=NODE_ID+'0';
	sprintf(test+21,"%d",trans_count);	
	
	test[25]=NODE_ID+'0';
	sprintf(test+26,"%d",trans_count);	
	
	test[30]=NODE_ID+'0';
	sprintf(test+31,"%d",trans_count);
	
	test[35]=NODE_ID+'0';
	sprintf(test+36,"%d",trans_count);	
	
	test[40]=NODE_ID+'0';
	sprintf(test+41,"%d",trans_count);	*/	
	


	#if PLATFORM_HAS_AGGREGATION
		ENERGEST_ON(ENERGEST_TYPE_SENSORS);
		if (get_num_payloads()>=1){
			payloadConcat(test);
			//avgPayload(test,(unsigned int) atoi(test));
			//printf("number of packets=%d\n",get_num_payloads());
		}
		ENERGEST_OFF(ENERGEST_TYPE_SENSORS);

	#endif

	memcpy(buffer,test,strlen(test)*sizeof(char));

  	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  	REST.set_response_payload(response, buffer, strlen(test));
  	/*reset has to be after operations to not alter values between for loops*/
  	reset_payloads();

  	/*This buffer print has data from other transmissions*/
  	#if RES_DEBUG
  	//printf("TRANSMISSION COUNT=%d  BUFFER=%s(len-%d)\n",trans_count,buffer,strlen(test));
  	printf("BUFFER=%s(%d) total trans=%d\n",buffer,strlen(test),trans_count);
	#endif

  	trans_count++;
}

static void
res_periodic_handler(){
  /* Do a periodic task here, e.g., sampling a sensor. */
  //++event_counter;
  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
  if(1) {
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_densenet);
  }
}
void 
payloadConcat(char * test){

	int i, total=0;
	for(i=0;i<get_num_payloads();i=i+1){
    	total+=snprintf(test+total,(LEN_SINGLE_PAYLOAD*MAX_N_PAYLOADS)-total,"%u",get_payloadAt(i));

    	//printf("test=%s\n",test );
	}


}

void 
maxPayload(char * test, unsigned int current){

	int i=0;
	unsigned int max=current;/*my number is the max value*/
	unsigned int temp=0;

	for(i=0;i<get_num_payloads();i=i+1){
		
		temp=get_payloadAt(i);
		
		if(max < temp )
			max = temp;

	}
	sprintf(test,"%u",max);

}
void 
minPayload(char * test,unsigned int current){
	
	int i=0;
	unsigned int min=current;
	unsigned int temp=0;

	for(i=0;i<get_num_payloads();i=i+1){
		
		temp=get_payloadAt(i);

		if(min > temp )
			min = temp;


	}
	sprintf(test,"%u",min);
}
void 
avgPayload(char * test,unsigned int current){

	int i=0;
	unsigned int number=current;
	printf("num1=%d\n",current );

	for(i=0;i<get_num_payloads();i=i+1){
		
		number += get_payloadAt(i);
	}
	printf("number=%u\n",number );
	number=number/(i+1);
	sprintf(test,"%u",number);

}
/*---------------------------------------------------------------------------*/