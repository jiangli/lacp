
typedef struct bpdu_header_t {
  unsigned char protocol[2];
  unsigned char version;
  unsigned char bpdu_type;
} BPDU_HEADER_T;

typedef struct lacpdu_t/* Lac_pdu */ /* only the relevant parameters */
{
	unsigned char  slow_protocols_address[6];
	unsigned char  src_address[6];
	unsigned short	ethertype;
	unsigned char protocol_subtype;
	unsigned char protocol_version;


	unsigned char type_actor;
	unsigned char len_actor;
	LAC_PORT_INFO  actor;
	unsigned char reverved1[3];
	
	unsigned char type_partner;
	unsigned char len_partner;
    LAC_PORT_INFO  partner;
	unsigned char reverved2[3];

	unsigned char type_collector;
	unsigned char len_collector;
	unsigned short collector_max_delay;
	unsigned char reverved3[12];
	
	unsigned char type_terminator;
	unsigned char len_terminator;
	unsigned char reverved4[50];
} LACPDU_T;

