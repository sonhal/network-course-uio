# IN4230 Mandatory Assignment 1

### Plan
create mipd - a MIP daemon for handling network requests

1. mipd starts up with a UNIX socket address clients can connect to - done
2. MIP-ARP - done
3. Update ARP-cache regularly - done
4. Drop the packet when a packet arrives and no application is connected on domain socket


**References:***
Utility libraries for handy data structures etc.

Linked list and Queue implementations where copied from
https://github.com/zedshaw/liblcthw/tree/master/src/lcthw under the following license: https://github.com/zedshaw/liblcthw/blob/master/LICENSE

Bstring library where copied from: http://bstring.sourceforge.net/

