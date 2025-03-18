# IRC Server

## Overview
This project is an **Internet Relay Chat (IRC) server** implemented in **C++98**.

It allows multiple clients to connect simultaneously using TCP/IP and communicate through text-based messages in real-time, supporting both public channels and private messaging.

Created in collaboration with Alonso Munoz (@agarrigu).

## Features
- **Multi-client support**: Handles multiple concurrent client connections without blocking or forking  
- **TCP/IP Communication**: Full support for IPv4 and IPv6 connections  
- **Authentication system**: Password protection for server access  
- **User management**: Nickname registration  
- **Channel operations**: Join channels, send and receive messages  
- **Operators**: Special commands available to channel operators only  

### Command Support:
- `PASS` - Authenticate with server password  
- `NICK` - Set or change nickname  
- `USER` - Specify username and real name  
- `JOIN` - Enter or create a channel  
- `PRIVMSG` - Send private messages to users or channels  
- `KICK` - Remove a user from a channel  
- `INVITE` - Invite a user to a channel  
- `TOPIC` - Set or view channel topics  
- `MODE` - Modify channel properties (*invite-only, topic restrictions, password, operator status, user limits*)  
- `PING` - Test server connection  

## Technical Requirements
- **C++98** compliant code
- **Non-blocking** I/O operations
- **Single `epoll()`** for all socket operations
- Compatible with **ircII**  

## Usage
```bash
./ircserv <port> <password>
```
- `port`: The port number on which the server listens for IRC connections
- `password`: The connection password

## Building
```bash
make        # Compile the project
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Rebuild the project from scratch
```

## Implementation Details
- All operations are non-blocking using `epoll()` for Linux and `kevent()` for MacOS
- Error handling covers network issues, client disconnections, and malformed commands
- No external libraries are used except standard C++98 libraries

## Testing
You can test basic functionality using `netcat`:
```bash
nc -C 127.0.0.1 <port>
```
For full testing, use an **ircII** client to connect to the server.

## Note
This is an educational project designed to understand network protocols and implement a server that complies with IRC standards while managing multiple concurrent connections efficiently.
