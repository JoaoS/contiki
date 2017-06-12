/**
 * \file
 *      payload header
 * \author
 *		João Subtil <jsubtil@student.dei.uc.pt>
 *		
 */

typedef enum {
	AGG_MIN_FUNC,
	AGG_AVG_FUNC,
	AGG_MAX_FUNC
}agg_func;


/*data from received payload*/
typedef struct{
	/*
	*4 bits groupID
	*4 bits agg function
	*4 bits number of nodes values included in the value
	*4 bits reserved for future
	*16 bit for the payload value
	**/
	uint8_t groupID;
	uint8_t agg_function;
	uint8_t total_num;
	uint8_t reserved;
	uint16_t value;

	unsigned int flag; // to check if this index has values
}singlePayload;


/*array of payloads to send*/
typedef struct {
 	int number_payloads; /*number of unique payloads in the buffer, the max equals*/
	singlePayload singleP[MAX_N_PAYLOADS];	/*space for each packet, every index corresponds to a groupID*/
}aggPayloads;




void reset_payloads(void);
singlePayload * ask_list(int index);
int payload_number(void);


/*specific funtions to handle the agg header*/
int parse_fix_agg_header(uint8_t * payload);
int check_for_my_group(uint8_t * payload, int nr_payloads);
void save_to_buffer(uint8_t * payload, int nr_payloads);

/*in group operations*/
uint16_t minPayload(uint16_t saved ,uint16_t current);
uint16_t avgPayload(uint16_t saved ,uint16_t current);
uint16_t maxPayload(uint16_t saved ,uint16_t current);


uint8_t count_packets(uint8_t * payload);
