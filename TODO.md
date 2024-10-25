```
 _____ ___  ____   ___
|_   _/ _ \|  _ \ / _ \
  | || | | | | | | | | |
  | || |_| | |_| | |_| |
  |_| \___/|____/ \___/.md
```
- [ ] connection:
  - [x] Accept client connections using TCP/IP
  - [x] Store information for each connected client
  - [ ] Handle client disconnections and timeouts
- [ ] authentication
- [ ] receive messages:
  - [ ] aggregate received packages
  - [x] parse message
  - [ ] commands:
    - [x] client (user):
      - [x] receive normal one-to-many to-a-group message
      - [x] `NICK` set (unique) nickname
      - [x] `USER` set username
      - [x] `JOIN` join channel
      - [x] `PRIVMSG` receive private message
    - [ ] client (oper):
      - [x] `KICK` kick client from channel
      - [x] `INVITE` invite client to channel
      - [x] `TOPIC` change/view client from channel
      - [ ] `MODE` set mode:
        - [ ] `i` set/remove invite-ony channel
        - [ ] `t` set/remove 'topic' change privilige
        - [ ] `k` set/remove channel key (password)
        - [ ] `o` give/take channel opperator priviledge
        - [ ] `l` set/remove user limit to channel
- [x] send messages:
  - [x] create message
  - [x] forward one-to-many to-a-group messages
  - [x] forward private messages
  - [x] send replies
- [ ] error checking:
  - [ ] partial data
  - [ ] low bandwidth
