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