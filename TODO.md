```
 _____ ___  ____   ___
|_   _/ _ \|  _ \ / _ \
  | || | | | | | | | | |
  | || |_| | |_| | |_| |
  |_| \___/|____/ \___/.md
```
- [ ] connection:
  - [ ] listen on port argv[1]
  - [ ] encode/decode TCP/IP packets
  - [ ] encode/decode TLS packets
- [ ] authentication
- [ ] receive messages:
  - [ ] aggregate received packages
  - [ ] parse message
  - [ ] commands:
    - [ ] client (user):
      - [ ] receive normal one-to-many to-a-group message
      - [ ] `/nick` set (unique) nickname
      - [ ] `/setname` set username
      - [ ] `/join` join channel
      - [ ] `/leave` leave channel
      - [ ] `/msg` receive private message
    - [ ] client (oper):
      - [ ] `/kick` kick client from channel
      - [ ] `/invite` invite client to channel
      - [ ] `/topic` change/view client from channel
      - [ ] `/mode` set mode:
        - [ ] `i` set/remove invite-ony channel
        - [ ] `t` set/remove 'topic' change privilige
        - [ ] `k` set/remove channel key (password)
        - [ ] `o` give/take channel opperator priviledge
        - [ ] `l` set/remove user limit to channel
- [ ] send messages:
  - [ ] create message
  - [ ] forward one-to-many to-a-group messages
  - [ ] forward private messages
  - [ ] send informative messages (kicked, invite, etc.)
- [ ] error checking:
  - [ ] partial data
  - [ ] low bandwidth
