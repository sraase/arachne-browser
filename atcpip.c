
// ========================================================================
// Arachne TCP/IP init
// (c)1997-1999 Michael Polak, Arachne Labs, xChaos software
// ========================================================================

#include "arachne.h"
#include "internet.h"

/* return local ip string */
char *atcp_get_ip_str(void)
{
#ifdef POSIX
	// modern systems often have multiple addresses
	// which can change dynamically per connection
	return "(auto)";
#else
	static char ipstr[16];
	sprintf(ipstr, "%u.%u.%u.%u",
		(uint8_t)(my_ip_addr >> 24), (uint8_t)(my_ip_addr >> 16),
		(uint8_t)(my_ip_addr >>  8), (uint8_t)(my_ip_addr >>  0));
	return ipstr;
#endif
}

/* return nameserver string */
char *atcp_get_dns_str(void)
{
#ifdef POSIX
	// name resolution on modern systems is complex,
	// just let the system resolver deal with it all
	return "(auto)";
#else
	static char ipstr[16];
	sprintf(ipstr, "%u.%u.%u.%u",
		(uint8_t)(def_nameservers[0] >> 24),
		(uint8_t)(def_nameservers[0] >> 16),
		(uint8_t)(def_nameservers[0] >>  8),
		(uint8_t)(def_nameservers[0] >>  0));
	return ipstr;
#endif
}

/* open new connection, returns 0 if successful */
int atcp_open(void *handle, const uint32_t *ip, uint16_t port)
{
#ifdef POSIX
	int sockfd, err;
	socklen_t errlen;
	struct sockaddr_in sin;

	// create non-blocking socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
		goto sock_err;

	// non-blocking connect
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = *ip;
	sin.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if (errno != EINPROGRESS)
			goto sock_err;
	}

	// idle while in progress
	while (1) {
		struct timeval tv;
		fd_set writefds;

		FD_ZERO(&writefds);
		FD_SET(sockfd, &writefds);
		tv.tv_sec  = 0;
		tv.tv_usec = 10000;
		if (select(sockfd + 1, NULL, &writefds, NULL, &tv) < 0) {
			if (errno != EINTR)
				goto sock_err;
		}

		if (FD_ISSET(sockfd, &writefds))
			break;

		if (TcpIdleFunc(NULL))
			goto sock_err;
	}

	// check if connect was successful
	errlen = sizeof(err);
	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
		goto sock_err;
	if (err) {
		errno = err;
		goto sock_err;
	}

	// connection successful
	*(int *)handle = sockfd;
	return 0;

sock_err:
	// connection failed
	close(sockfd);
	return -1;
#else
	tcp_Socket *socket = (tcp_Socket *)handle;
	int status;

	// non-blocking connect
	status = tcp_open(socket, locport(), *ip, port, NULL);
	if (!status)
		goto sock_err;

	// idle while in progress
	sock_wait_established(socket, sock_delay, TcpIdleFunc, &status);

	// connection successful
	return 0;

sock_err:
	// connection failed
	return -1;
#endif
}

/* close connection */
void atcp_close(void *handle)
{
#ifdef POSIX
	int sockfd = *(int *)handle;
	close(sockfd);
#else
	tcp_Socket *socket = (tcp_Socket *)handle;
	sock_close(socket);
#endif
}

/* send (and flush) data, returns 0 if successful */
int atcp_send(void *handle, const char *buf, size_t len)
{
#ifdef POSIX
	int sockfd = *(int *)handle;
	while (len) {
		ssize_t l = send(sockfd, buf, len, 0);
		if (l < 0)
			return -1;

		buf += l;
		len -= l;

		if (TcpIdleFunc(NULL))
			return -1;
	}
	return 0;
#else
	tcp_Socket *socket = (tcp_Socket *)handle;
	while (len) {
		int l = sock_write(socket, (byte *)buf, (int)len);
		if (!l)
			return -1;

		buf += l;
		len -= l;

		if (TcpIdleFunc(NULL))
			return -1;
	}

	sock_flush(socket);
	return 0;
#endif
}

/* receive data
     return number of bytes read
     return 0 if no data available OR connection closed
     return -1 on error */
int atcp_recv(void *handle, char *buf, size_t len)
{
#ifdef POSIX
	int sockfd = *(int *)handle;
	while (1) {
		ssize_t ret = recv(sockfd, buf, len, 0);
		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 0; // no data
		}
		return (int)ret;
	}
#else
	tcp_Socket *socket = (tcp_Socket *)handle;
	int ret = sock_fastread(socket, (byte *)buf, (int)len);
	return ret;
#endif
}

/* check if data available
     return 0  if no data available
     return >0 if data available OR error */
int atcp_has_data(void *handle)
{
#ifdef POSIX
	int ret, sockfd = *(int *)handle;
	struct timeval tv;
	fd_set efds, rfds;

	while (1) {
		tv.tv_sec = 0;  tv.tv_usec = 500;
		FD_ZERO(&rfds); FD_SET(sockfd, &rfds);
		FD_ZERO(&efds); FD_SET(sockfd, &efds);
		ret = select(sockfd + 1, &rfds, NULL, &efds, &tv);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return 1; // error
		}

		if (FD_ISSET(sockfd, &rfds) || FD_ISSET(sockfd, &efds))
			return 1; // data available or closed

		return 0;
	}
#else
	tcp_Socket *socket = (tcp_Socket *)handle;
	int ret = sock_dataready(socket);
	return (ret == 0) ? 0 : 1;
#endif
}

#ifdef POSIX
static volatile int      atcp_resolve_valid;
static volatile uint32_t atcp_resolve_result;

static void *atcp_resolve_thread(void *arg)
{
	const char *hostname = (const char *)arg;
	struct hostent *he = gethostbyname(hostname);
	if (!he || !he->h_addr_list[0]) {
		atcp_resolve_result = 0;
		atcp_resolve_valid  = 1;
	} else {
		atcp_resolve_result = *(uint32_t*)he->h_addr_list[0];
		atcp_resolve_valid  = 1;
	}
	return NULL;
}
#endif

/* resolve hostname, returns 0 if successful */
int atcp_resolve(const char *hostname, uint32_t *ip)
{
#ifdef POSIX
	pthread_t thread;

	atcp_resolve_valid = 0;
	if (pthread_create(&thread, NULL, atcp_resolve_thread, (void *)hostname))
		return -1;

	while (!atcp_resolve_valid) {
		if (TcpIdleFunc(NULL)) {
			pthread_cancel(thread);
			return -1;
		}
	}

	if (atcp_resolve_result) {
		*ip = atcp_resolve_result;
		atcp_resolve_valid = 0;
		return 0;
	}

	atcp_resolve_valid = 0;
	return -1;
#else
	longword result = resolve_fn((char *)hostname, TcpIdleFunc);
	if (result) {
		*ip = (uint32_t)result;
		return 0;
	}
	return -1;
#endif
}

#ifndef POSIX
void errppp(void)
{
 tcpip=0;
 if(arachne.scriptline)
 {
  strcpy(GLOBAL.location,"gui:err_asf.ah");
  arachne.scriptline=0; //disable scripting!
 }
 else
 {
  strcpy(GLOBAL.location,"gui:err_ppp.ah");
  pagetime=time(NULL);
 }
}

void ArachneTCPIP(void)
{
 char dialer=0;
 char *value=config_get_str("Connection", NULL);

//!!glennmcc: Mar 06 2006
char *dns="\0", *dns1="\0", *dns2="\0";
//!!glennmcc: end

 if(!value)
 {
  tcpip=0;
  return;
 }

 if(value[0]=='@')
  dialer=1;

 if(!strncmpi(value,"READY",5))
  tcpip=1;
 else
 if(!strncmpi(value,"HTTPSTUB",8))
  httpstub=1;
 else
 if(!tcpip && dialer && !GLOBAL.location[0])
 {
  value = config_get_str("DialPage", "file:ppp_init.htm");
  strcpy(GLOBAL.location,value);
 }

 if(tcpip) //inicializace TCP/IP pokud je -o, -r ...
 {
  outs(MSG_TCPIP);

  value = config_get_str("TCPconfig", NULL);
  if(value)
   tcp_config_file( value);

  value = config_get_str("IP_Address", NULL);
  if(!value)
  {
   tcpip=0;
   strcpy(GLOBAL.location,"gui:err_noip.ah");
   return;
  }

  if(!strncmpi(value,"PPP",3))
   ipmode=MODE_PPP;
  else if(!strncmpi(value,"BOOTP",5))
   ipmode=MODE_BOOTP;
  else if(!strncmpi(value,"WATTCP",5))
   ipmode=MODE_WATTCP;

//  _bootpon = 0;
  switch(ipmode)
  {
   case MODE_NORMAL:

    if(value[0]=='%' || value[0]=='$' )
    {
     char *ptr;
     //enviroment
     makestr(myIPstr,&value[1],19);
     //vymazani zaverecneho '%'
     ptr=strchr(&myIPstr[1],'%');
     if(ptr)
      *ptr='\0';
     makestr(myIPstr,getenv(myIPstr),19);
    }
    else
     makestr(myIPstr,value,19);

    my_ip_addr = resolve( myIPstr );
    break;

   case MODE_BOOTP:
    //_bootpon = 1;
    tcp_config_file( NULL );
    //!!JdS: 2003/12/7 {
    //  If a dial-up connection, initialize 'ppplogtime' via PPPlog().
    //  This allows the "Time Online" status display to function.
    //  Note that PPPlog() has been rewritten to avoid side-effects.
    //!!JdS: 2003/12/7 }
    if(dialer)
     PPPlog();  //failure is tolerable here
    break;

   case MODE_PPP:
    if(!PPPlog())
    {
     errppp();
     return;
    }
  }

  if(sock_init_noexit())
  {
   if(dialer)
    errppp();
   else
   {
    tcpip=0;
    strcpy(GLOBAL.location,"gui:err_pkt.ah");
   }
   return;
  }
  else
   tcpip=1;

/*
  websocket=malloc(sizeof(tcp_Socket));
  if(websocket)
  {
   tcp_listen(websocket, 80, 0, 0, webserver , 0);
  }
*/


  if(ipmode==MODE_PPP)
   loadrefresh=500;

  if(ipmode!=MODE_WATTCP && ipmode!=MODE_BOOTP)
  {
   value = config_get_str("Gateway", NULL);
   if(value) _arp_add_gateway( value , 0L );
   value = config_get_str("AltGateway", NULL);
   if(value) _arp_add_gateway( value , 0L );
   value = config_get_str("Netmask", NULL);
   if(value) sin_mask = resolve( value );
  }

//!!glennmcc: Mar 06 2006 -- modified entire section to use 'dns1' and 'dns2'
//instead of the original 'value' in all places
  dns1 = config_get_str("NameServer", NULL);
//!!glennmcc: Dec 12, 2005 -- add NameServer %DNS1% capability
  if(dns1 && *dns1=='%')
    {
     char *ptr=strchr(&dns1[1],'%');
     if(ptr) *ptr='\0';
     dns=getenv(&dns1[1]);
    }
  else strcpy(dns,dns1);
//!!glennmcc: end
  if(dns) _add_server( &_last_nameserver, MAX_NAMESERVERS, def_nameservers, resolve(dns));
  dns2 = config_get_str("AltNameServer", NULL);
//!!glennmcc: Dec 12, 2005 -- add AltNameServer %DNS2% capability
  if(dns2 && *dns2=='%')
    {
     char *ptr=strchr(&dns2[1],'%');
     if(ptr) *ptr='\0';
     dns=getenv(&dns2[1]);
    }
  else strcpy(dns,dns2);
//!!glennmcc: end
  if(dns) _add_server( &_last_nameserver, MAX_NAMESERVERS, def_nameservers, resolve(dns));
 }//endif tcp/ip
}
#endif
