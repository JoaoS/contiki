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

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "lib/random.h"
#include "net/ip/agg_payloads.h"
#include <node-id.h>

/*subtil*/
#include "sys/clock.h"


#define MAX_N_PAYLOADS 40
#define LEN_SINGLE_PAYLOAD 2


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);
static int trans_count=1;
static int reverse(int number);


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
	int length=1;
	int j=0;
	char * value;
	char agg[MAX_N_PAYLOADS];
	int i;
	
	#if PLATFORM_HAS_AGGREGATION
		for(i=0,j=0;i<MAX_N_PAYLOADS;i=i+1){
			agg[i]='\0';	
		}

	    #if DENSENET_DEBUG
		printf("--- Number of payloads is %d \n",get_num_payloads());
		#endif

		for(i=0,j=0;i<get_num_payloads();i=i+1){
			value=get_payload_char(i);
			agg[j]=value[0];
			j++;
			agg[j]=value[1]; 
			j++;
		}
		length+=(2*get_num_payloads());
	#endif

	
	/*Each message has node id + tansmission count**/
	agg[j]=NODE_ID+'0';
	j++;
	int diff=j;
	int score=trans_count;

	/*avoid eliminating the zeros iN 10, 20 etc...*/
	if (!(score % 10==0))
		score=reverse(trans_count);
	
	while(score){
	    agg[j]= (score % 10)+'0';
	    #if DENSENET_DEBUG
	    printf("agg[%d]=%c\n",j,agg[j] );
	    #endif
	    j++;
	    score /= 10;
	}
	length+=(j-diff);


	char neuf='9';
	sprintf((char *)buffer, "%c", neuf);
	strncpy((char*)buffer,agg,length*sizeof(char));
	reset_payloads();

	#if DENSENET_DEBUG
	printf("Parsing:Buffer is %s, length is %d \n",buffer, length);
	#endif

  	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
 	//REST.set_header_etag(response, (uint8_t *)&length, 1);
 	//REST.set_response_payload(response, buffer, length);
  	REST.set_response_payload(response, buffer, length);
  	printf("TRANSMISSION COUNT=%d\n",trans_count);
  	trans_count++;



}

static int
reverse(int number){
	int  reverse = 0, rightDigit;
	while(number != 0){
        rightDigit = number % 10;
        reverse = (reverse * 10) + rightDigit;
        number = number/10;
    }

   return reverse;

}


static void
res_periodic_handler()
{
  /* Do a periodic task here, e.g., sampling a sensor. */
  //++event_counter;

  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
  if(1) {
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_densenet);
  }
}
