mikro
=====

Files:
* library:
  * eth0 - ethernet frame structure
  * ipv6 - ipv6 and icmp headers, parsing functions, sending ndp packets
  * ndp - implementation of NDP:
    * send ndp solicitation
    * send ndp advertisement (solicited or unsolicited)
    * update mac table
    * searching mac table for given IP address
    * performing actions ndp solicitation/advertisement/mac table update, depending on the contents of received ICMP packet
  * tcp
* tests:
  * test_ndp - tests sending ICMP/NDP packets
  * test_geteth0 - tests receiving and parsing frames
  * test_eth0 - tests sending raw ethernet frames
