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
void add_payload(char *);
char * get_payloads();
int get_num_payloads();
char * get_payload_char(int payload_position);
