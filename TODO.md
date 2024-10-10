```
 _____ ___  ____   ___
|_   _/ _ \|  _ \ / _ \
  | || | | | | | | | | |
  | || |_| | |_| | |_| |
  |_| \___/|____/ \___/.md
```
- [ ] connection:
  - [ ] Accept client connections using TCP/IP
  - [ ] Store information for each connected client
  - [ ] Handle client disconnections and timeouts
- [ ] authentication
- [ ] receive messages:
  - [ ] aggregate received packages
  - [ ] parse message
  - [ ] commands:
    - [ ] client (user):
      - [ ] receive normal one-to-many to-a-group message
      - [ ] `NICK` set (unique) nickname
      - [ ] `USER` set username
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
  - [ ] create message
  - [ ] forward one-to-many to-a-group messages
  - [ ] forward private messages
  - [ ] send replies
- [ ] error checking:
  - [ ] partial data
  - [ ] low bandwidth
