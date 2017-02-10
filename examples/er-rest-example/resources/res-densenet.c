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


/*subtil*/
#include "sys/clock.h"


#define MAX_N_PAYLOADS 40
#define LEN_SINGLE_PAYLOAD 4 
#define MAX_INT 9999
#define RES_DEBUG 1



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);
static int trans_count=100;
int payloadConcat(char * test, int totalsize);
int maxPayload(char * test);
int minPayload(char * test);
int avgPayload(char * test);
//static int reverse(int number);


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
         10*CLOCK_SECOND,
         res_periodic_handler);
/*20 seconds =255*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	/*
	*	This is where we set the packet payload and annex other payloads if available
	*	first we should check if packets are available for processing, 
	*/

	char test[MAX_N_PAYLOADS];
	int totalsize=0,i=0;
	for(i=0;i<MAX_N_PAYLOADS;i=i+1){test[i]='\0';}

	
	totalsize = payloadConcat(test, totalsize);
	
	/*
	totalsize = avgPayload(test);
	totalsize = maxPayload(test);
	totalsize = minPayload(test);
	*/
	
	/*Each message has node id + tansmission count**/
	test[totalsize]=NODE_ID+'0';
	totalsize++;
	sprintf(test+totalsize,"%d",trans_count);
	//add size of current counter to totalsize

	/**FIXME the log 10 does not work, count digits*/
	totalsize+=log10(trans_count);	
	totalsize++;
	//printf("count=%d, log=%lf, size=%d\n",trans_count,log10(trans_count), totalsize );
	/*printf("BEFORE TRANSMISSION COUNT=%d\nBUFFER=%s\n",trans_count,buffer);*/
	/*clean buffer between msg results n reboots, yay!*/
	//memset(&buffer,0,sizeof(buffer));
	memcpy(buffer,test,totalsize*sizeof(char));

	/*reset has to be after operations to not alter values between for loops*/
	reset_payloads();
  	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  	REST.set_response_payload(response, buffer, totalsize);

  	/*This buffer print has data from other transmissions*/
  	printf("TRANSMISSION COUNT=%d  BUFFER=%s(%d)\n",trans_count,buffer,totalsize);

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


int 
payloadConcat(char * test, int totalsize){

	#if PLATFORM_HAS_AGGREGATION
		int i=0;


		#if RES_DEBUG
		printf("--- Number of payloads is %d ---\n",get_num_payloads());
		#endif
		/**/
		for(i=0;i<get_num_payloads();i=i+1){

			if(i!=0)
				strncpy((char *)test+totalsize,(char *)get_payload_char(i),LEN_SINGLE_PAYLOAD);
			else
				strncpy((char *)test,(char *)get_payload_char(i),LEN_SINGLE_PAYLOAD);

			printf("paychar=%s\n",get_payload_char(i) );	
			totalsize+=LEN_SINGLE_PAYLOAD;

		}
		#if RES_DEBUG
		printf("MERGER: test=%s, size=%d\n",test,totalsize );
		#endif

		return totalsize;

	#endif
}

int 
avgPayload(char * test){

	int i=0;
	unsigned int totalValues=0;
	int size=0;

	for(i=0;i<get_num_payloads();i=i+1){

		totalValues+=get_payloads(i);
		//printf("payloads=%d\n",get_payloads(i) );
		size=get_pay_len(i);

	}
	totalValues=totalValues/get_num_payloads();
	sprintf(test,"%d",totalValues);
	//printf("AVG char=%d, size avg=%d, test=%s\n",(char*)totalValues,sizeof(totalValues),test);

	/*return number of digits*/
	return size;
}

int 
maxPayload(char * test){

	int i=0;
	unsigned int temp=0;
	int size=0;


	for(i=0;i<get_num_payloads();i=i+1){

		if(temp < get_payloads(i))
			temp = get_payloads(i);

		size=get_pay_len(i);

	}
	sprintf(test,"%d",temp);

	/*return number of digits*/
	return size;
}

int 
minPayload(char * test){

	int i=0;
	unsigned int temp=MAX_INT;
	int size=0;


	for(i=0;i<get_num_payloads();i=i+1){

		if(temp > get_payloads(i))
			temp = get_payloads(i);

		size=get_pay_len(i);


	}
	sprintf(test,"%d",temp);

	/*return number of digits*/
	return size;
}






































/*---------------------------------------------------------------------------*/