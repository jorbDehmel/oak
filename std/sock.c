/*
The C side of an Oak POSIX socket interface
Jordan Dehmel, 2023
jdehmel@outlook.com
*/

#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>

#include "/usr/include/oak/std_oak_header.h"

struct string
{
    i8 *data;
    u128 size;
};

////////////////////////////////////////////////////////////////
// Server socket definitions
////////////////////////////////////////////////////////////////

// Must be 64 bytes long
struct server_sock
{
    int id, client_id;
    int addr_size;
    struct sockaddr_in address;

    char padding[36];
};

void New_FN_PTR_server_sock_MAPS_void(struct server_sock *self)
{
    memset(self, 0, 32);
}

void Del_FN_PTR_server_sock_MAPS_void(struct server_sock *self)
{
    if (self->id != 0)
    {
        shutdown(self->id, SHUT_RDWR);
    }
    memset(self, 0, 32);
}

// Copies in data, but does NOT begin listening yet
struct server_sock Copy_FN_PTR_server_sock_JOIN_str_JOIN_u16_JOIN_PTR_i32_MAPS_server_sock(
    struct server_sock *self, str addr, u16 port, i32 *result)
{
    int yes = 1;

    // Set options for IPv4
    memset(&self->address, 0, sizeof(self->address));
    self->address.sin_family = AF_INET;

    *result = inet_pton(AF_INET, addr, &(self->address.sin_addr));

    if (*result != 1)
    {
        puts("inet_pton error");
        return *self;
    }

    self->address.sin_port = htons(port); // converts to needed endianness
    self->addr_size = sizeof(self->address);

    // Create socket
    self->id = socket(AF_INET, SOCK_STREAM, 0);

    if (self->id == -1)
    {
        puts("socket error");
        *result = -1;
        return *self;
    }

    // Set socket options
    *result = setsockopt(self->id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (*result)
    {
        puts("setsockopt error");
        return *self;
    }

    // Bind
    *result = bind(self->id, (struct sockaddr *)(&self->address), sizeof(self->address));

    return *self;
}

i32 listen_FN_PTR_server_sock_MAPS_i32(struct server_sock *self)
{
    int status = 0;

    status = listen(self->id, 5);

    if (status)
    {
        return status;
    }

    self->client_id = accept(self->id, (struct sockaddr *)(&self->address), (socklen_t *)(&self->addr_size));

    return 0;
}

struct string recv_FN_PTR_server_sock_JOIN_u128_MAPS_string(struct server_sock *self, u128 size)
{
    struct string out;
    out.size = size + 1;
    out.data = (i8 *)malloc(sizeof(i8) * out.size);
    memset(out.data, '\0', out.size);

    recv(self->client_id, out.data, size, 0);

    return out;
}

i32 send_FN_PTR_server_sock_JOIN_string_MAPS_i32(struct server_sock *self, struct string what)
{
    return send(self->client_id, what.data, what.size, 0);
}

void close_FN_PTR_server_sock_MAPS_void(struct server_sock *self)
{
    if (self->id != 0)
    {
        shutdown(self->id, SHUT_RDWR);
    }
    memset(self, 0, 32);
}

////////////////////////////////////////////////////////////////
// Client socket definitions
////////////////////////////////////////////////////////////////

struct client_sock
{
    int id;
    int addr_size;
    struct sockaddr_in address;

    char padding[40];
};

void New_FN_PTR_client_sock_MAPS_void(struct client_sock *self)
{
    memset(self, 0, 32);
}

void Del_FN_PTR_client_sock_MAPS_void(struct client_sock *self)
{
    if (self->id != 0)
    {
        shutdown(self->id, SHUT_RDWR);
    }
    memset(self, 0, 32);
}

// Copies in data, but does NOT begin listening yet
struct client_sock Copy_FN_PTR_client_sock_JOIN_str_JOIN_u16_JOIN_PTR_i32_MAPS_client_sock(
    struct client_sock *self, str addr, u16 port, i32 *result)
{
    int yes = 1;

    // Set options for IPv4
    memset(&self->address, 0, sizeof(self->address));
    self->address.sin_family = AF_INET;

    *result = inet_pton(AF_INET, addr, &(self->address.sin_addr));

    if (*result != 1)
    {
        puts("inet_pton error");
        return *self;
    }

    self->address.sin_port = htons(port); // converts to needed endianness
    self->addr_size = sizeof(self->address);

    // Create socket
    self->id = socket(AF_INET, SOCK_STREAM, 0);

    // Set socket options
    *result = setsockopt(self->id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (*result)
    {
        puts("setsockopt error");
        return *self;
    }

    // Connect
    *result = connect(self->id, (struct sockaddr *)(&self->address), self->addr_size);

    if (*result)
    {
        puts("connect error");
    }

    return *self;
}

struct string recv_FN_PTR_client_sock_JOIN_u128_MAPS_string(struct client_sock *self, u128 size)
{
    struct string out;
    out.size = size + 1;
    out.data = (i8 *)malloc(sizeof(i8) * out.size);
    memset(out.data, '\0', out.size);

    recv(self->id, out.data, size, 0);

    return out;
}

i32 send_FN_PTR_client_sock_JOIN_string_MAPS_i32(struct client_sock *self, struct string what)
{
    return send(self->id, what.data, what.size, 0);
}

void close_FN_PTR_client_sock_MAPS_void(struct client_sock *self)
{
    if (self->id != 0)
    {
        shutdown(self->id, SHUT_RDWR);
    }
    memset(self, 0, 32);
}
