#define DEBUG

// user-defined
#include "headers.h"
#include "gtrace.h"

// C++
#include <fstream>
#include <vector>
#include <string>
#include <utility>

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// network
#include <pcap.h>

char errbuf[PCAP_ERRBUF_SIZE];

#pragma pack(push, 1)
struct _tcpPacket final {
	struct _eth eth;
	struct _ip  ip;
	struct _tcp tcp;
};
#pragma pack(pop)

void usage() {
	printf("usage: sudo ./block-packet <mirror interface> <send interface>\n");
	printf("example: sudo ./block-packet enp0s3 eth0\n");
}

pcap_t* open_pcap(char* interface) {
	pcap_t* pcap = pcap_open_live(interface, BUFSIZ, 1, 1, errbuf);
	if(pcap == NULL) {
		fprintf(stderr, "pcap_open_live(%s) return null - %s\n", interface, errbuf);
		return NULL;
	}
	return pcap;
}

uint8_t hex2int(char n) {
	return ('0' <= n && n <= '9') ? n - '0' : n - 'a' + 10;
}

uint8_t* resolve_mac(char* interface) {
	std::ifstream iface("/sys/class/net/" + std::string(interface) + "/address");
	std::string mac((std::istreambuf_iterator<char>(iface)), std::istreambuf_iterator<char>());
	mac.pop_back();
	uint8_t *ret = (uint8_t*)malloc(sizeof(uint8_t) * 8);
	for(int i=0; i<6; i++) {
		ret[i] = hex2int(mac[i*3])*16 + hex2int(mac[i*3+1]);
	}
	return ret;
}

bool custom_filter(_tcpPacket *pkt) {
	if(pkt->eth.type() != _eth::ipv4) return true;
	if(pkt->ip.proto() != _ip::tcp) return true;
	// if(pkt->tcp.flags2() != _tcp::flags_psh | _tcp::flags_ack) return true;
	return false;
}

int main(int argc, char* argv[]) {
	GTRACE("Debug mode is enabled");

	if(argc != 3) {
		usage();
		return -1;
	}
	char* mirror_interface = argv[1];
	char* send_interface = argv[2];

	pcap_t* mirror_pcap = open_pcap(mirror_interface);
	if(mirror_pcap == NULL) {
		return -1;
	}
	GTRACE("mirror pcap is created");

	uint8_t *my_mac = resolve_mac(send_interface);

	// begin of raw socket
	int send_socket = ::socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if(send_socket == -1) {
		GTRACE("socket return -1");
		return -1;
	}

	int one = 1;
	int* pone = &one;
	int res = ::setsockopt(send_socket, IPPROTO_IP, IP_HDRINCL, pone, sizeof(one));
	if(res < 0) {
		GTRACE("setsockopt(IP_HDRINCL) return FAIL");
		return -1;
	}

	if(send_interface != "") {
		res = ::setsockopt(send_socket, SOL_SOCKET, SO_BINDTODEVICE, send_interface, strlen(send_interface));
		if(res < 0) {
			GTRACE("setsockopt(SO_BINDTODEVICE) return FAIL");
			return -1;
		}
	}

	struct sockaddr_in addr_in_;
	memset(&addr_in_, 0, sizeof(addr_in_));
	addr_in_.sin_family = AF_INET;

	GTRACE("send socket is created");
	// end of raw socket

	int pkt_cnt = 0;
	_tcpPacket *tcpPacket = new _tcpPacket;
	_tcpPacket *fwd = new _tcpPacket;
	_tcpPacket *bwd = new _tcpPacket;
	struct pcap_pkthdr* header;
	const uint8_t* packet;
	while(++pkt_cnt) {
		res = pcap_next_ex(mirror_pcap, &header, &packet);
		if(res == 0) continue;

		// initialize
		tcpPacket->eth = *((struct _eth*)(packet));
		tcpPacket->ip  = *((struct _ip* )(packet + ETH_SIZE));
		tcpPacket->tcp = *((struct _tcp*)(packet + ETH_SIZE + tcpPacket->ip.ip_size()));

		// you can modify custom_filter() function
		// it must return true, when a packet is recieved what you don't need
		if(custom_filter(tcpPacket)) continue;

		// copy packet
		fwd->eth = bwd->eth = tcpPacket->eth;
		fwd->ip  = bwd->ip  = tcpPacket->ip;
		fwd->tcp = bwd->tcp = tcpPacket->tcp;

		// modify mac address
		for(int i=0; i<6; i++)
			fwd->eth._src[i] = bwd->eth._src[i] = my_mac[i];
		for(int i=0; i<6; i++)
			bwd->eth._dst[i] = tcpPacket->eth._src[i];

		// modify ip header
		fwd->ip._len = bwd->ip._len = ntohs(tcpPacket->ip.ip_size() + tcpPacket->tcp.tcp_size());
		std::swap(bwd->ip._src, bwd->ip._dst);
		bwd->ip._ttl = 128;

		// modify tcp header
		std::swap(bwd->tcp._srcport, bwd->tcp._dstport);
		fwd->tcp._seq_raw = ntohl(tcpPacket->tcp.seq_raw() + _tcp::len(&(tcpPacket->ip), &(tcpPacket->tcp)));
		bwd->tcp._seq_raw = tcpPacket->tcp._seq_raw;
		fwd->tcp._flags2 = bwd->tcp._flags2 = _tcp::flags_ack | _tcp::flags_rst;

		// calculate ip and tcp checksum
		fwd->ip._checksum = _ip::calcIpChecksum(&(fwd->ip));
		bwd->ip._checksum = _ip::calcIpChecksum(&(bwd->ip));

		fwd->tcp._checksum = _tcp::calcTcpChecksum(&(fwd->ip), &(fwd->tcp));
		bwd->tcp._checksum = _tcp::calcTcpChecksum(&(bwd->ip), &(bwd->tcp));


		// send packet
		addr_in_.sin_addr.s_addr = fwd->ip._dst;
		res = ::sendto(send_socket, &(fwd->ip), fwd->ip.len(), 0, (struct sockaddr*)&addr_in_, sizeof(struct sockaddr_in));
		GTRACE("send forward packet");

		addr_in_.sin_addr.s_addr = bwd->ip._dst;
		res = ::sendto(send_socket, &(bwd->ip), bwd->ip.len(), 0, (struct sockaddr*)&addr_in_, sizeof(struct sockaddr_in));
		GTRACE("send backward packet");

		// print counter
		printf("========%d========", pkt_cnt);
	}

	delete tcpPacket, fwd, bwd;
	pcap_close(mirror_pcap);
	::close(send_socket);
	
	return 0;
}
