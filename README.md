mikro
=====

Files:
* library:
  * eth0 - ethernet frame structure
  * ipv6 - ipv6 and icmp headers, parsing functions, sending ndp packets, ipv6 checksum calculation
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
  * test_getndp - tests replying to NDP Solicitation and saving addresses to MAC table
  * test_tcp - tests sending TCP packets
  * test_get - tests receiving TCP packets, and replying to SYN packets
