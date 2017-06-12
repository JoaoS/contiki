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

#define RES_DEBUG 0
#define N_VALUES 3 //number of uint8_t elements of packets, 1 fixed +2 information and 2(uint16_t) for the payload



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);
static int trans_count=GROUPID*1000+NODE_ID;


void payloadConcat(char * test);

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
         SEND_INTERVAL*CLOCK_SECOND,
         res_periodic_handler);
/*20 seconds =255*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{	
	/*check if i have data to send*/
	
	#if PRODUCER
		uint8_t  my_packet[N_VALUES]={0};
		uint8_t fixed_header,var;
		uint16_t payload=0;
		
		fixed_header=var=0;

		/*create my own header*/
		fixed_header |= EXTERNAL_CONCAT << FIX_AGG_HEADER_ALLOW_CONCAT_POSITION;
		fixed_header |= 1 << FIX_AGG_HEADER_NRPAYLOADS_POSITION;				//producer only produces one packet
		my_packet[0] = fixed_header;


		/*now create the unit payload*/
		/*fullfil first byte of my packet, groupid and aggregation function*/
		var |= GROUPID << GROUPID_POSITION;
		var |= AGG_FUNCTION << AGG_FUNCTION_POSITION;
		my_packet[1] = var;

		var=0;
		var |= 1 << NRVALUES_POSITION;
		my_packet[2] = var;

		payload=trans_count;//the actual payload value

		/*copy all data to buffer*/
		memcpy(buffer,&my_packet,N_VALUES*sizeof(uint8_t));
		memcpy(buffer+N_VALUES*sizeof(uint8_t),&payload,sizeof(uint16_t));
		REST.set_response_payload(response, buffer, 5);

	#else

		#if PLATFORM_HAS_AGGREGATION
			ENERGEST_ON(ENERGEST_TYPE_SENSORS);
			/*go through buffer and add all the data*/
			int nr_units=payload_number();
			int flag=0;
			uint8_t fixed_header=0;
			uint8_t byte_one, byte_two;
			uint16_t payload=0;

			
			fixed_header |= EXTERNAL_CONCAT << FIX_AGG_HEADER_ALLOW_CONCAT_POSITION;
			fixed_header |= nr_units << FIX_AGG_HEADER_NRPAYLOADS_POSITION;				//producer only produces one packet
			memcpy(buffer,&fixed_header,sizeof(uint8_t));//put the fixed header in buffer

			int i=0;
			if (nr_units)//if i have packets to send
			{
				for (; i < MAX_N_PAYLOADS; ++i)
				{
					singlePayload * iterator=ask_list(i);
					if (iterator->flag)
					{
						//create unit
						//printf(" iteration number=%d\n",i);
						byte_one=byte_two=0;

						byte_one |= (iterator->groupID) << GROUPID_POSITION;
						byte_one |= (iterator->agg_function) << AGG_FUNCTION_POSITION;
						
						byte_two |= (iterator->total_num) << NRVALUES_POSITION;
						#if 1
							printf("\nnr de valores-res/densenet=%u, iterador=%u\n",byte_two,(iterator->total_num) );
						#endif

						payload=(iterator->value);

						memcpy(buffer+(1*sizeof(uint8_t))+(4*flag*sizeof(uint8_t)),&byte_one,sizeof(uint8_t)); //byte one
						memcpy(buffer+(1*sizeof(uint8_t))+(1*sizeof(uint8_t))+(4*flag*sizeof(uint8_t)),&byte_two,sizeof(uint8_t));//byte two
						memcpy(buffer+(1*sizeof(uint8_t))+(2*sizeof(uint8_t))+(4*flag*sizeof(uint8_t)),&payload,sizeof(uint16_t));//bytes 3 and 4
						flag++;
						
					}
					else
						continue;
				}
  			REST.set_response_payload(response, buffer, 1+(flag)*4);
			}
			else{
				REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
				REST.set_response_payload(response, buffer, 0);
			}
			ENERGEST_OFF(ENERGEST_TYPE_SENSORS);
		#endif
	#endif


	
  	reset_payloads();

  	/*This buffer print has data from other transmissions*/
  	#if RES_DEBUG
  	  	printf("BUFFER=%u total trans=%d\n",buffer,trans_count);
	#endif

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
/*---------------------------------------------------------------------------*/