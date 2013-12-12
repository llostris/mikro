mikro
=====

Files:
* library:
  * eth0 - ethernet frame structure
  * ipv6 - ipv6 and icmp headers, parsing functions, sending ndp packets
  * ndp - implementation of NDP
  * tcp
* tests:
  * test_ndp - tests sending ICMP/NDP packets
  * test_geteth0 - tests receiving and parsing frames
  * test_eth0 - tests sending raw ethernet frames
