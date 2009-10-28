struct __IPV6_HEADER{
	DWORD        dwVersion      : 4;
	DWORD        dwTrafficClass : 8;
	DWORD        dwFlowLabel    : 20;
	DWORD        dwPayLoadLen   : 16;
	DWORD        dwNextHeader   : 8;
	DWORD        dwHopLimit     : 8;
	UCHAR        SourceAddress[16];
	UCHAR        DestinationAddress[16];
	UCHAR        ExtendHeader[0];
};

