#define getsockname hack_getsockname
#define sockaddr hack_sockaddr
#define in_addr hack_in_addr
/*
 *
 * Ethernet Interface
 */

extern byte *_eth_FormatPacket(), *_eth_WaitPacket();

typedef struct ether {
    byte	dest[6];
    byte	src[6];
    word	type;
    byte	data[ 1500 ];
};


#define ETH_MIN	60              /* Minimum Ethernet packet size */
