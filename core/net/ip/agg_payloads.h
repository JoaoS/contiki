/**
 * \file
 *      payload header
 * \author
 *      André Riker <ariker@dei.uc.pt>
 *		João Subtil <jsubtil@dei.uc.pt>
 *		
 */
struct singlePayload;
struct aggPayloads;
void reset_payloads();

/**/
void add_payload(uint8_t *incomingPayload);
unsigned int get_payloadAt(int payload_position);
int get_num_payloads();


/*specific funtions to handle the agg header*/
void parse_fix_agg_header(uint8_t * payload);

/*
*1 bit to check if external concat is enabled
*3 reserved bits
*4 bits to the number os payloads
*/
typedef struct {

	uint8_t external_concat;
	/*discard the 3 reserved  bits*/
	uint8_t nr_payloads;
	
}agg_fixed_header_t;


/*
typedef struct {

	
}agg_unit_header_t;*/