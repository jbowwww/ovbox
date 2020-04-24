#include "udpsocket.h"
#include "common.h"
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <strings.h>

#define LISTEN_BACKLOG 512

udpsocket_t::udpsocket_t()
{
  bzero((char*)&serv_addr, sizeof(serv_addr));
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
    throw ErrMsg("Opening socket failed: ", errno);
}

udpsocket_t::~udpsocket_t()
{
  close(sockfd);
}

void udpsocket_t::destination(const char* host)
{
  struct hostent* server;
  server = gethostbyname(host);
  if(server == NULL)
    throw ErrMsg("No such host: " + std::string(hstrerror(h_errno)));
  bzero((char*)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
        server->h_length);
}

void udpsocket_t::bind(uint32_t port)
{
  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval,
             sizeof(int));

  endpoint_t my_addr;
  memset(&my_addr, 0, sizeof(endpoint_t));
  /* Clear structure */
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons((unsigned short)port);

  if(::bind(sockfd, (struct sockaddr*)&my_addr, sizeof(endpoint_t)) == -1)
    throw ErrMsg("Binding the socket to port " + std::to_string(port) +
                     " failed: ",
                 errno);
}

size_t udpsocket_t::send(const char* buf, size_t len, int portno)
{
  if(portno == 0)
    return len;
  serv_addr.sin_port = htons(portno);
  return sendto(sockfd, buf, len, MSG_CONFIRM, (struct sockaddr*)&serv_addr,
                sizeof(serv_addr));
}

size_t udpsocket_t::send(const char* buf, size_t len, const endpoint_t& ep )
{
  return sendto(sockfd, buf, len, MSG_CONFIRM, (struct sockaddr*)&ep,
                sizeof(ep));
}

size_t udpsocket_t::recvfrom(char* buf, size_t len, endpoint_t& addr)
{
  memset(&addr, 0, sizeof(endpoint_t));
  addr.sin_family = AF_INET;
  socklen_t socklen(sizeof(endpoint_t));
  return ::recvfrom(sockfd, buf, len, 0, (struct sockaddr*)&addr, &socklen);
}

std::string addr2str( const struct in_addr& addr )
{
  return std::to_string(addr.s_addr & 0xff) + "." +
    std::to_string((addr.s_addr>>8) & 0xff) + "." +
    std::to_string((addr.s_addr>>16) & 0xff) + "." +
    std::to_string((addr.s_addr>>24) & 0xff);
}

std::string ep2str( const endpoint_t& ep )
{
  return addr2str( ep.sin_addr ) + "/" + std::to_string( ntohs( ep.sin_port) );
}