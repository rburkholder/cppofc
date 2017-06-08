# cppofc
C++ based openflow packet encoder/decoder

* focusses on decoding openflow 1.4.1 spec packets for use with openvswitch
* anticpated use case: 
** make use of logcabin/raft to create a distributed controller
** use distributed network minmax algorithms based upon link bandwidth to manage routes and traffic
** load a linux distribution onto Mellanox SN2700 switches, and make use of the hardware offload functionality currently in development for the kernel and openvswitch
** control hypervisors for traffic encapsulation and function chaining
** ie: a distributed solution running on hypervisors, routers, and switches for managing traffic end to end
