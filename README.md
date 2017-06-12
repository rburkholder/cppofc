# cppofc
C++ based openflow packet encoder/decoder

* a focus on decoding openflow v1.4.1 protocol packets for use with openvswitch
* being a distributed solution running on hypervisors, routers, and switches for managing traffic end to end
* anticpated use case: 
  * make use of logcabin/raft to create a distributed controller
  * use distributed network minmax algorithms based upon link bandwidth to manage routes and traffic
  * load a linux distribution onto Mellanox SN2700 switches, and make use of the hardware offload functionality currently in development for the kernel and openvswitch
  * control hypervisors for traffic encapsulation and function chaining
