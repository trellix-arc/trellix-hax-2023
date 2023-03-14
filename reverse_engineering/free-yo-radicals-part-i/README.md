# Free_Yo_Radicals_Part_i 

### 200 Points

## Description
A long-range drone was found in the mountains on the edge of the habitable zone by hazmat-suited scouts. This drone is equipped with multiple scientific testing devices, including a radiation sensor. The working theory is that this drone is a part of a fleet used to identify habitable areas by logging radiation levels across what remains of the continental U.S. The onboard computer is still intact and the team has recovered what seems to be an old packet capture, possibly used during development.

Our spy across the exclusion zone was unable to obtain the source code for the drone, but managed to leak the binary for the server-side application. The client software was corrupt, so your task is to reverse engineer the communication protocol and attempt to interact with the live server. Intel we've gathered strongly suggests that the protocol utilizes a strange and elaborate "handshake" that will require you to send over one of every type of packet supported by the protocol. Only then will the server send back the Fully Linked Authentication GUID (FLAG) in a packet.

## Hint
The hint is in the file "challenge/.hint"
