//
// Created by ameerg on 15/06/2022.
//
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

/* READING THE DATA INTO THE SOCKET */
int read_data (int s, char *buf, int n)
{
  int bcount;
  int br;
  bcount = 0;
  br = 0;
  while (bcount < n)
    {
      br = read (s, buf, n - bcount);
      if (br > 0)
        {
          bcount += br;
          buf += br;
        }
      if (br < 1)
        {
          return -1;
        }
    }
  return bcount;
}

/* THE CLIENT SOCKET FUNCTIONS */
int call_socket (char *hostname, int port_num)
{
  struct sockaddr_in sa;
  struct hostent *hp;
  int s = 0;

  if ((hp = gethostbyname (hostname)) == nullptr)
    {
      return -1;
    }

  memset (&sa, 0, sizeof (sa));
  memcpy ((char *) &sa.sin_addr, hp->h_addr_list[0], hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short) port_num);
  if ((s = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
    {
      return -1;
    }
//    write (s,command, strlen (command));
  std::cout << sizeof (sa) << std::endl;
  if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {
      close (s);
      return -1;
    }

  return s;
}

/* THE SERVER SIDE FUNCTIONS */
int get_connection (int socket)
{
  int t;
  if ((t = accept (socket, nullptr, nullptr)) < 0)
    {
      return -1;
    }
  return t;
}

int establish (unsigned short port_num)
{
  char my_name[256];
  struct sockaddr_in sock_addr_in;
  int s;
  struct hostent *hp;
  gethostname (my_name, 255);

  hp = gethostbyname (my_name);
  if (hp == nullptr)
    {
      return -1;
    }
  memset (&sock_addr_in, 0, sizeof (struct sockaddr_in));
  sock_addr_in.sin_family = hp->h_addrtype;
  memcpy (&sock_addr_in.sin_addr, hp->h_addr_list[0], hp->h_length);
  sock_addr_in.sin_port = htons(port_num);
  s = socket (AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    {
      return -1;
    }
  int x = bind (s, (struct sockaddr *) &sock_addr_in, sizeof (struct sockaddr_in));
  if (x < 0)
    {
      close (s);
      return -1;
    }

  if (listen (s, 5) < 0)
    {

      return -1;
    }
  while (true)
    {
      int connection = get_connection (s);
      if (connection == -1)
        {

          return -1;
        }
      char buffer[256] = {0};
      int read_num = read_data (connection, buffer, 256);
      system (buffer);
    }

  return s;
}

/* READING THE COMMAND FROM ARGV */
void get_command (char *arr[], int argc, char *command)
{
  for (int i = 3; i < argc; ++i)
    {
      strcat (command, arr[i]);
      strcat (command, " ");
    }
}

int main (int argc, char *argv[])
{
  int ser;
  int cl;
  // dealing with the server socket
  if (strcmp (argv[1], "server") == 0)
    {
      int port_num = (int) strtol (argv[2], nullptr, 10);
      ser = establish (port_num);
      if (ser == -1)
        {
          exit (1);
        }
    }
  // dealing with the client socket
  if (strcmp (argv[1], "client") == 0)
    {
      char command[256] = {0};
      get_command (argv, argc, command);
      int port_num = (int) strtol (argv[2], nullptr, 10);
      char hostname[256];
      gethostname (hostname, 256);
      cl = call_socket (hostname, port_num);
      if (cl == -1)
        {
          return -1;
        }
      if (send (cl, command, 256, 0) == -1)
        {
          exit (1);
        }
      close (cl);
    }
  close (ser);
}
