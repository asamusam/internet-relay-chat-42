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
    - [ ] client (user):
      - [x] receive normal one-to-many to-a-group message
      - [x] `NICK` set (unique) nickname
      - [x] `USER` set username
      - [ ] `JOIN` join channel
      - [ ] `PART` leave channel
      - [ ] `PRIVMSG` receive private message
    - [ ] client (oper):
      - [ ] `KICK` kick client from channel
      - [ ] `INVITE` invite client to channel
      - [ ] `TOPIC` change/view client from channel
      - [ ] `MODE` set mode:
        - [ ] `i` set/remove invite-ony channel
        - [ ] `t` set/remove 'topic' change privilige
        - [ ] `k` set/remove channel key (password)
        - [ ] `o` give/take channel opperator priviledge
        - [ ] `l` set/remove user limit to channel
- [ ] send messages:
  - [x] create message
  - [ ] forward one-to-many to-a-group messages
  - [ ] forward private messages
  - [x] send replies
- [ ] error checking:
  - [ ] partial data
  - [ ] low bandwidth
