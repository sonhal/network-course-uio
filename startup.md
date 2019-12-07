# Networking

Will be about what networks *really* are

### Network architecture

- Recursive architecture
    Scope


###  Path MTU discovery
- Disable fragmentation
Find the largest datagram that can be sent.

Black hole detection
 - Packet is lost, no way to know why

PLPMTUD: Start small


### Multi path transfere

TCP makes many assumptions
Little reordering, almost no loss
- IP tries to be natural. yet, new in-network mechanisms must not break TCP assumptions

### Non-techincal reasons for problems

- Compromises

- Opposing incentives
    Google, Cisco, Netgear, Telenor

### People do agree on some things

- End-to-end argument
    - Carefully consider trade-offs between implementing functions inside the network end systems
    - Think twice before letting the network do it
    - No application-specific functionality in the network
    - Frequently interpreted too strictly:
        *"Keep the network dumb, put all the logic in end systems"*

- Postel's robustness principle *"Be conservative in what you send, be liberal in what you accept"*
- Ensures downward comparability (e.g. "reserved" bits)

### ...But they still fight
Pervasive Monitoring is an Attack - RFC 7258

Solution: Encrypt everything, end-to-end, QUIC

Also: Ossification
- Middleboxen makes assumptions and drop new advancements in protocols

