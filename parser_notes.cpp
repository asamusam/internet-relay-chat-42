/*
The server to which a client is connected is required to parse the
complete message, returning any appropriate errors. If the server
encounters a fatal error while parsing a message, an error must be
sent back to the client and the parsing terminated.  A fatal error
may be considered to be incorrect command, a destination which is
otherwise unknown to the server (server, nick or channel names fit
this category), not enough parameters or incorrect privileges.

If a full set of parameters is presented, then each must be checked
for validity and appropriate responses sent back to the client.  In
the case of messages which use parameter lists using the comma as an
item separator, a reply must be sent for each item.
*/

/*
"Clients should not use prefix when sending a message from
themselves; if they use a prefix, the only valid prefix is the
registered nickname associated with the client.  If the source
identified by the prefix cannot be found from the server's internal
database, or if the source is registered from a different link than
from which the message arrived, the server must ignore the message
silently".
*/

/*
The numeric reply must be sent as one
message consisting of the sender prefix, the three digit numeric, and
the target of the reply.
*/

/*
A numeric reply is not allowed to originate
from a client; any such messages received by a server are silently
dropped.
*/


/* JOIN
To create a new channel or become part of an existing channel, a user
is required to JOIN the channel.  If the channel doesn't exist prior
to joining, the channel is created and the creating user becomes a
channel operator.  If the channel already exists, whether or not your
request to JOIN that channel is honoured depends on the current modes
of the channel. For example, if the channel is invite-only, (+i),
then you may only join if invited.  As part of the protocol, a user
may be a part of several channels at once, but a limit of ten (10)
channels is recommended as being ample for both experienced and
novice users.
*/