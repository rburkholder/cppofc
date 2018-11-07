# cppofc
C++ based openflow packet encoder/decoder

* a focus on decoding openflow v1.4.1 protocol packets for use with openvswitch
* being a distributed solution running on hypervisors, routers, and switches for managing traffic end to end
* anticpated use case: 
  * make use of logcabin/raft to create a distributed controller
  * use distributed network minmax algorithms based upon link bandwidth to manage routes and traffic
  * load a linux distribution onto Mellanox SN2700 switches, and make use of the hardware offload functionality currently in development for the kernel and openvswitch
  * control hypervisors for traffic encapsulation and function chaining

# Build boost:
```
wget https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.gz
tar zxvf boost_1_65_1.tar.gz
cd boost_1_65_1
sh booststrap.sh
sudo ./b2 --layout=versioned variant=release link=shared threading=multi runtime-link=shared install
```

# A test setup:
```
# show existing service
ovs-vsctl show
# add two namespaces
ip netns add left
ip netns add right
# turn up lo in the namespaces
ip netns exec left ip link set dev lo up
ip netns exec right ip link set dev lo up
# add a bridge
ovs-vsctl add-br ovsbr0
# set controller of the bridge for openflow
ovs-vsctl set-fail-mode ovsbr0 secure
ovs-vsctl set-controller ovsbr0 tcp:0.0.0.0:6633
# bridge needs to be turned up as that is how it handles some stp stuff
ip link set dev ovsbr0 up
ip link set dev ovs-system up
# add two ports
ovs-vsctl add-port ovsbr0 ethleft -- set interface ethleft type=internal
ovs-vsctl add-port ovsbr0 ethright -- set interface ethright type=internal
# move the two interfaces into the namespaces
ip link set dev ethleft netns left
ip link set dev ethright netns right
# turn the interfaces up
ip netns exec right ip link set dev ethright up
ip netns exec left ip link set dev ethleft up
# add addresses to the interfaces
ip netns exec left ip addr add dev ethleft 172.16.1.1/24
ip netns exec right ip addr add dev ethright 172.16.1.2/24
# confirm connectivity with a ping
ip netns exec right ping 172.16.1.1
```


# Dump Flows:

    ovs-ofctl dump-flows ovsbr0

# Dump rules:

    ovs-appctl dpif/dump-flows ovsbr0
