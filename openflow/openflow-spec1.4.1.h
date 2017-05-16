/* Copyright (c) 2008 The Board of Trustees of The Leland Stanford
 * Junior University
 * Copyright (c) 2011, 2013 Open Networking Foundation
 *
 * We are making the OpenFlow specification and associated documentation
 * (Software) available for public use and benefit with the expectation
 * that others will use, modify and enhance the Software and contribute
 * those enhancements back to the community. However, since we would
 * like to make the Software available for broadest use, with as few
 * restrictions as possible permission is hereby granted, free of
 * charge, to any person obtaining a copy of this Software to deal in
 * the Software under the copyrights without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The name and trademarks of copyright holder(s) may NOT be used in
 * advertising or publicity pertaining to the Software or any
 * derivatives without specific, written prior permission.
 */

/* OpenFlow: protocol between controller and datapath. */

#ifndef OPENFLOW_OPENFLOW_H
#define OPENFLOW_OPENFLOW_H 1

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#ifdef SWIG
#define OFP_ASSERT(EXPR)        /* SWIG can't handle OFP_ASSERT. */
#elif !defined(__cplusplus)
/* Build-time assertion for use in a declaration context. */
#define OFP_ASSERT(EXPR)                                                \
        extern int (*build_assert(void))[ sizeof(struct {               \
                    unsigned int build_assert_failed : (EXPR) ? 1 : -1; })]
#else /* __cplusplus */
#define OFP_ASSERT(_EXPR) typedef int build_assert_failed[(_EXPR) ? 1 : -1]
#endif /* __cplusplus */

#ifndef SWIG
#define OFP_PACKED __attribute__((packed))
#else
#define OFP_PACKED              /* SWIG doesn't understand __attribute. */
#endif

/* Version number:
 * OpenFlow versions released: 0x01 = 1.0 ; 0x02 = 1.1 ; 0x03 = 1.2
 *     0x04 = 1.3.X ; 0x05 = 1.4.X
 */
/* The most significant bit in the version field is reserved and must
 * be set to zero.
 */
#define OFP_VERSION   0x05

#define OFP_MAX_TABLE_NAME_LEN 32
#define OFP_MAX_PORT_NAME_LEN  16

/* Official IANA registered port for OpenFlow. */
#define OFP_TCP_PORT  6653
#define OFP_SSL_PORT  6653

#define OFP_ETH_ALEN 6          /* Bytes in an Ethernet address. */

/* Port numbering. Ports are numbered starting from 1. */
enum ofp_port_no {
    /* Maximum number of physical and logical switch ports. */
    OFPP_MAX        = 0xffffff00,

    /* Reserved OpenFlow Port (fake output "ports"). */
    OFPP_IN_PORT    = 0xfffffff8,  /* Send the packet out the input port.  This
                                      reserved port must be explicitly used
                                      in order to send back out of the input
                                      port. */
    OFPP_TABLE      = 0xfffffff9,  /* Submit the packet to the first flow table
                                      NB: This destination port can only be
                                      used in packet-out messages. */
    OFPP_NORMAL     = 0xfffffffa,  /* Forward using non-OpenFlow pipeline. */
    OFPP_FLOOD      = 0xfffffffb,  /* Flood using non-OpenFlow pipeline. */
    OFPP_ALL        = 0xfffffffc,  /* All standard ports except input port. */
    OFPP_CONTROLLER = 0xfffffffd,  /* Send to controller. */
    OFPP_LOCAL      = 0xfffffffe,  /* Local openflow "port". */
    OFPP_ANY        = 0xffffffff   /* Special value used in some requests when
                                      no port is specified (i.e. wildcarded). */
};

enum ofp_type {
    /* Immutable messages. */
    OFPT_HELLO              = 0,  /* Symmetric message */
    OFPT_ERROR              = 1,  /* Symmetric message */
    OFPT_ECHO_REQUEST       = 2,  /* Symmetric message */
    OFPT_ECHO_REPLY         = 3,  /* Symmetric message */
    OFPT_EXPERIMENTER       = 4,  /* Symmetric message */

    /* Switch configuration messages. */
    OFPT_FEATURES_REQUEST   = 5,  /* Controller/switch message */
    OFPT_FEATURES_REPLY     = 6,  /* Controller/switch message */
    OFPT_GET_CONFIG_REQUEST = 7,  /* Controller/switch message */
    OFPT_GET_CONFIG_REPLY   = 8,  /* Controller/switch message */
    OFPT_SET_CONFIG         = 9,  /* Controller/switch message */

    /* Asynchronous messages. */
    OFPT_PACKET_IN          = 10, /* Async message */
    OFPT_FLOW_REMOVED       = 11, /* Async message */
    OFPT_PORT_STATUS        = 12, /* Async message */

    /* Controller command messages. */
    OFPT_PACKET_OUT         = 13, /* Controller/switch message */
    OFPT_FLOW_MOD           = 14, /* Controller/switch message */
    OFPT_GROUP_MOD          = 15, /* Controller/switch message */
    OFPT_PORT_MOD           = 16, /* Controller/switch message */
    OFPT_TABLE_MOD          = 17, /* Controller/switch message */

    /* Multipart messages. */
    OFPT_MULTIPART_REQUEST      = 18, /* Controller/switch message */
    OFPT_MULTIPART_REPLY        = 19, /* Controller/switch message */

    /* Barrier messages. */
    OFPT_BARRIER_REQUEST    = 20, /* Controller/switch message */
    OFPT_BARRIER_REPLY      = 21, /* Controller/switch message */

    /* Controller role change request messages. */
    OFPT_ROLE_REQUEST       = 24, /* Controller/switch message */
    OFPT_ROLE_REPLY         = 25, /* Controller/switch message */

    /* Asynchronous message configuration. */
    OFPT_GET_ASYNC_REQUEST  = 26, /* Controller/switch message */
    OFPT_GET_ASYNC_REPLY    = 27, /* Controller/switch message */
    OFPT_SET_ASYNC          = 28, /* Controller/switch message */

    /* Meters and rate limiters configuration messages. */
    OFPT_METER_MOD          = 29, /* Controller/switch message */

    /* Controller role change event messages. */
    OFPT_ROLE_STATUS        = 30, /* Async message */

    /* Asynchronous messages. */
    OFPT_TABLE_STATUS       = 31, /* Async message */

    /* Request forwarding by the switch. */
    OFPT_REQUESTFORWARD     = 32, /* Async message */

    /* Bundle operations (multiple messages as a single operation). */
    OFPT_BUNDLE_CONTROL     = 33, /* Controller/switch message */
    OFPT_BUNDLE_ADD_MESSAGE = 34, /* Controller/switch message */
};

/* Header on all OpenFlow packets. */
struct ofp_header {
    big_uint8_t version;    /* OFP_VERSION. */
    big_uint8_t type;       /* One of the OFPT_ constants. */
    big_uint16_t length;    /* Length including this ofp_header. */
    big_uint32_t xid;       /* Transaction id associated with this packet.
                           Replies use the same id as was in the request
                           to facilitate pairing. */
};
OFP_ASSERT(sizeof(struct ofp_header) == 8);

/* Hello elements types.
 */
enum ofp_hello_elem_type {
    OFPHET_VERSIONBITMAP          = 1,  /* Bitmap of version supported. */
};

/* Common header for all Hello Elements */
struct ofp_hello_elem_header {
    big_uint16_t         type;    /* One of OFPHET_*. */
    big_uint16_t         length;  /* Length in bytes of the element,
                                 including this header, excluding padding. */
};
OFP_ASSERT(sizeof(struct ofp_hello_elem_header) == 4);

/* Version bitmap Hello Element */
struct ofp_hello_elem_versionbitmap {
    big_uint16_t         type;    /* OFPHET_VERSIONBITMAP. */
    big_uint16_t         length;  /* Length in bytes of this element,
                                 including this header, excluding padding. */
    /* Followed by:
     *   - Exactly (length - 4) bytes containing the bitmaps, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         bitmaps[0];   /* List of bitmaps - supported versions */
};
OFP_ASSERT(sizeof(struct ofp_hello_elem_versionbitmap) == 4);

/* OFPT_HELLO.  This message includes zero or more hello elements having
 * variable size. Unknown elements types must be ignored/skipped, to allow
 * for future extensions. */
struct ofp_hello {
    struct ofp_header header;

    /* Hello element list */
    struct ofp_hello_elem_header elements[0]; /* List of elements - 0 or more */
};
OFP_ASSERT(sizeof(struct ofp_hello) == 8);

#define OFP_DEFAULT_MISS_SEND_LEN   128

enum ofp_config_flags {
    /* Handling of IP fragments. */
    OFPC_FRAG_NORMAL   = 0,       /* No special handling for fragments. */
    OFPC_FRAG_DROP     = 1 << 0,  /* Drop fragments. */
    OFPC_FRAG_REASM    = 1 << 1,  /* Reassemble (only if OFPC_IP_REASM set). */
    OFPC_FRAG_MASK     = 3,       /* Bitmask of flags dealing with frag. */
};

/* Switch configuration. */
struct ofp_switch_config {
    struct ofp_header header;
    big_uint16_t flags;             /* Bitmap of OFPC_* flags. */
    big_uint16_t miss_send_len;     /* Max bytes of packet that datapath
                                   should send to the controller. See
                                   ofp_controller_max_len for valid values.
                                   */
};
OFP_ASSERT(sizeof(struct ofp_switch_config) == 12);

/* Table numbering. Tables can use any number up to OFPT_MAX. */
enum ofp_table {
    /* Last usable table number. */
    OFPTT_MAX        = 0xfe,

    /* Fake tables. */
    OFPTT_ALL        = 0xff   /* Wildcard table used for table config,
                                 flow stats and flow deletes. */
};

/* Flags to configure the table. */
enum ofp_table_config {
    OFPTC_DEPRECATED_MASK    = 3,       /* Deprecated bits */
    OFPTC_EVICTION           = 1 << 2,  /* Authorise table to evict flows. */
    OFPTC_VACANCY_EVENTS     = 1 << 3,  /* Enable vacancy events. */
};

/* Table Mod property types.
 */
enum ofp_table_mod_prop_type {
    OFPTMPT_EVICTION               = 0x2,    /* Eviction property. */
    OFPTMPT_VACANCY                = 0x3,    /* Vacancy property. */
    OFPTMPT_EXPERIMENTER           = 0xFFFF, /* Experimenter property. */
};

/* Common header for all Table Mod Properties */
struct ofp_table_mod_prop_header {
    big_uint16_t         type;    /* One of OFPTMPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_table_mod_prop_header) == 4);

/* Eviction flags. */
enum ofp_table_mod_prop_eviction_flag {
    OFPTMPEF_OTHER           = 1 << 0,     /* Using other factors. */
    OFPTMPEF_IMPORTANCE      = 1 << 1,     /* Using flow entry importance. */
    OFPTMPEF_LIFETIME        = 1 << 2,     /* Using flow entry lifetime. */
};

/* Eviction table mod Property. Mostly used in OFPMP_TABLE_DESC replies. */
struct ofp_table_mod_prop_eviction {
    big_uint16_t         type;    /* OFPTMPT_EVICTION. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         flags;   /* Bitmap of OFPTMPEF_* flags */
};
OFP_ASSERT(sizeof(struct ofp_table_mod_prop_eviction) == 8);

/* Vacancy table mod property */
struct ofp_table_mod_prop_vacancy {
    big_uint16_t         type;   /* OFPTMPT_VACANCY. */
    big_uint16_t         length; /* Length in bytes of this property. */
    big_uint8_t vacancy_down;    /* Vacancy threshold when space decreases (%). */
    big_uint8_t vacancy_up;      /* Vacancy threshold when space increases (%). */
    big_uint8_t vacancy;         /* Current vacancy (%) - only in ofp_table_desc. */
    big_uint8_t pad[1];          /* Align to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_table_mod_prop_vacancy) == 8);

/* Experimenter table mod property */
struct ofp_table_mod_prop_experimenter {
    big_uint16_t         type;    /* OFPTMPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_table_mod_prop_experimenter) == 12);

/* Configure/Modify behavior of a flow table */
struct ofp_table_mod {
    struct ofp_header header;
    big_uint8_t table_id;       /* ID of the table, OFPTT_ALL indicates all tables */
    big_uint8_t pad[3];         /* Pad to 32 bits */
    big_uint32_t config;        /* Bitmap of OFPTC_* flags */

    /* Table Mod Property list */
    struct ofp_table_mod_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_table_mod) == 16);

/* Capabilities supported by the datapath. */
enum ofp_capabilities {
    OFPC_FLOW_STATS     = 1 << 0,  /* Flow statistics. */
    OFPC_TABLE_STATS    = 1 << 1,  /* Table statistics. */
    OFPC_PORT_STATS     = 1 << 2,  /* Port statistics. */
    OFPC_GROUP_STATS    = 1 << 3,  /* Group statistics. */
    OFPC_IP_REASM       = 1 << 5,  /* Can reassemble IP fragments. */
    OFPC_QUEUE_STATS    = 1 << 6,  /* Queue statistics. */
    OFPC_PORT_BLOCKED   = 1 << 8,  /* Switch will block looping ports. */
    OFPC_BUNDLES        = 1 << 9,  /* Switch supports bundles. */
    OFPC_FLOW_MONITORING = 1 << 10,  /* Switch supports flow monitoring. */
};

/* Flags to indicate behavior of the physical port.  These flags are
 * used in ofp_port to describe the current configuration.  They are
 * used in the ofp_port_mod message to configure the port's behavior.
 */
enum ofp_port_config {
    OFPPC_PORT_DOWN    = 1 << 0,  /* Port is administratively down. */

    OFPPC_NO_RECV      = 1 << 2,  /* Drop all packets received by port. */
    OFPPC_NO_FWD       = 1 << 5,  /* Drop packets forwarded to port. */
    OFPPC_NO_PACKET_IN = 1 << 6   /* Do not send packet-in msgs for port. */
};

/* Current state of the physical port.  These are not configurable from
 * the controller.
 */
enum ofp_port_state {
    OFPPS_LINK_DOWN    = 1 << 0,  /* No physical link present. */
    OFPPS_BLOCKED      = 1 << 1,  /* Port is blocked */
    OFPPS_LIVE         = 1 << 2,  /* Live for Fast Failover Group. */
};

/* Features of ports available in a datapath. */
enum ofp_port_features {
    OFPPF_10MB_HD    = 1 << 0,  /* 10 Mb half-duplex rate support. */
    OFPPF_10MB_FD    = 1 << 1,  /* 10 Mb full-duplex rate support. */
    OFPPF_100MB_HD   = 1 << 2,  /* 100 Mb half-duplex rate support. */
    OFPPF_100MB_FD   = 1 << 3,  /* 100 Mb full-duplex rate support. */
    OFPPF_1GB_HD     = 1 << 4,  /* 1 Gb half-duplex rate support. */
    OFPPF_1GB_FD     = 1 << 5,  /* 1 Gb full-duplex rate support. */
    OFPPF_10GB_FD    = 1 << 6,  /* 10 Gb full-duplex rate support. */
    OFPPF_40GB_FD    = 1 << 7,  /* 40 Gb full-duplex rate support. */
    OFPPF_100GB_FD   = 1 << 8,  /* 100 Gb full-duplex rate support. */
    OFPPF_1TB_FD     = 1 << 9,  /* 1 Tb full-duplex rate support. */
    OFPPF_OTHER      = 1 << 10, /* Other rate, not in the list. */

    OFPPF_COPPER     = 1 << 11, /* Copper medium. */
    OFPPF_FIBER      = 1 << 12, /* Fiber medium. */
    OFPPF_AUTONEG    = 1 << 13, /* Auto-negotiation. */
    OFPPF_PAUSE      = 1 << 14, /* Pause. */
    OFPPF_PAUSE_ASYM = 1 << 15  /* Asymmetric pause. */
};

/* Port description property types.
 */
enum ofp_port_desc_prop_type {
    OFPPDPT_ETHERNET          = 0,      /* Ethernet property. */
    OFPPDPT_OPTICAL           = 1,      /* Optical property. */
    OFPPDPT_EXPERIMENTER      = 0xFFFF, /* Experimenter property. */
};

/* Common header for all port description properties. */
struct ofp_port_desc_prop_header {
    big_uint16_t         type;    /* One of OFPPDPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_port_desc_prop_header) == 4);

/* Ethernet port description property. */
struct ofp_port_desc_prop_ethernet {
    big_uint16_t         type;    /* OFPPDPT_ETHERNET. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint8_t          pad[4];  /* Align to 64 bits. */
    /* Bitmaps of OFPPF_* that describe features.  All bits zeroed if
     * unsupported or unavailable. */
    big_uint32_t curr;          /* Current features. */
    big_uint32_t advertised;    /* Features being advertised by the port. */
    big_uint32_t supported;     /* Features supported by the port. */
    big_uint32_t peer;          /* Features advertised by peer. */

    big_uint32_t curr_speed;    /* Current port bitrate in kbps. */
    big_uint32_t max_speed;     /* Max port bitrate in kbps */
};
OFP_ASSERT(sizeof(struct ofp_port_desc_prop_ethernet) == 32);

/* Features of optical ports available in switch. */
enum ofp_optical_port_features {
    OFPOPF_RX_TUNE   = 1 << 0,  /* Receiver is tunable */
    OFPOPF_TX_TUNE   = 1 << 1,  /* Transmit is tunable */
    OFPOPF_TX_PWR    = 1 << 2,  /* Power is configurable */
    OFPOPF_USE_FREQ  = 1 << 3,  /* Use Frequency, not wavelength */
};

/* Optical port description property. */
struct ofp_port_desc_prop_optical {
    big_uint16_t         type;    /* OFPPDPT_3OPTICAL. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint8_t          pad[4];  /* Align to 64 bits. */

    big_uint32_t supported;     /* Features supported by the port. */
    big_uint32_t tx_min_freq_lmda;   /* Minimum TX Frequency/Wavelength */
    big_uint32_t tx_max_freq_lmda;   /* Maximum TX Frequency/Wavelength */
    big_uint32_t tx_grid_freq_lmda;  /* TX Grid Spacing Frequency/Wavelength */
    big_uint32_t rx_min_freq_lmda;   /* Minimum RX Frequency/Wavelength */
    big_uint32_t rx_max_freq_lmda;   /* Maximum RX Frequency/Wavelength */
    big_uint32_t rx_grid_freq_lmda;  /* RX Grid Spacing Frequency/Wavelength */
    big_uint16_t tx_pwr_min;         /* Minimum TX power */
    big_uint16_t tx_pwr_max;         /* Maximum TX power */
};
OFP_ASSERT(sizeof(struct ofp_port_desc_prop_optical) == 40);

/* Experimenter port description property. */
struct ofp_port_desc_prop_experimenter {
    big_uint16_t         type;    /* OFPPDPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_port_desc_prop_experimenter) == 12);

/* Description of a port */
struct ofp_port {
    big_uint32_t port_no;
    big_uint16_t length;
    big_uint8_t pad[2];
    big_uint8_t hw_addr[OFP_ETH_ALEN];
    big_uint8_t pad2[2];                  /* Align to 64 bits. */
    char name[OFP_MAX_PORT_NAME_LEN]; /* Null-terminated */

    big_uint32_t config;        /* Bitmap of OFPPC_* flags. */
    big_uint32_t state;         /* Bitmap of OFPPS_* flags. */

    /* Port description property list - 0 or more properties */
    struct ofp_port_desc_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_port) == 40);

/* Switch features. */
struct ofp_switch_features {
    struct ofp_header header;
    big_uint64_t datapath_id;   /* Datapath unique ID.  The lower 48-bits are for
                               a MAC address, while the upper 16-bits are
                               implementer-defined. */

    big_uint32_t n_buffers;     /* Max packets buffered at once. */

    big_uint8_t n_tables;       /* Number of tables supported by datapath. */
    big_uint8_t auxiliary_id;   /* Identify auxiliary connections */
    big_uint8_t pad[2];         /* Align to 64-bits. */

    /* Features. */
    big_uint32_t capabilities;  /* Bitmap of support "ofp_capabilities". */
    big_uint32_t reserved;
};
OFP_ASSERT(sizeof(struct ofp_switch_features) == 32);

/* What changed about the physical port */
enum ofp_port_reason {
    OFPPR_ADD     = 0,         /* The port was added. */
    OFPPR_DELETE  = 1,         /* The port was removed. */
    OFPPR_MODIFY  = 2,         /* Some attribute of the port has changed. */
};

/* A physical port has changed in the datapath */
struct ofp_port_status {
    struct ofp_header header;
    big_uint8_t reason;          /* One of OFPPR_*. */
    big_uint8_t pad[7];          /* Align to 64-bits. */
    struct ofp_port desc;
};
OFP_ASSERT(sizeof(struct ofp_port_status) == 56);

/* Port mod property types.
 */
enum ofp_port_mod_prop_type {
    OFPPMPT_ETHERNET          = 0,      /* Ethernet property. */
    OFPPMPT_OPTICAL           = 1,      /* Optical property. */
    OFPPMPT_EXPERIMENTER      = 0xFFFF, /* Experimenter property. */
};

/* Common header for all port mod properties. */
struct ofp_port_mod_prop_header {
    big_uint16_t         type;    /* One of OFPPMPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_port_mod_prop_header) == 4);

/* Ethernet port mod property. */
struct ofp_port_mod_prop_ethernet {
    big_uint16_t      type;       /* OFPPMPT_ETHERNET. */
    big_uint16_t      length;     /* Length in bytes of this property. */
    big_uint32_t      advertise;  /* Bitmap of OFPPF_*.  Zero all bits to prevent
                                 any action taking place. */
};
OFP_ASSERT(sizeof(struct ofp_port_mod_prop_ethernet) == 8);

struct ofp_port_mod_prop_optical {
    big_uint16_t      type;       /* OFPPMPT_OPTICAL. */
    big_uint16_t      length;     /* Length in bytes of this property. */
    big_uint32_t      configure;  /* Bitmap of OFPOPF_*. */
    big_uint32_t      freq_lmda;  /* The "center" frequency */
    big_int32_t       fl_offset;  /* signed frequency offset */
    big_uint32_t      grid_span;  /* The size of the grid for this port */
    big_uint32_t      tx_pwr;     /* tx power setting */
};
OFP_ASSERT(sizeof(struct ofp_port_mod_prop_optical) == 24);

/* Experimenter port mod property. */
struct ofp_port_mod_prop_experimenter {
    big_uint16_t         type;    /* OFPPMPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_port_mod_prop_experimenter) == 12);

/* Modify behavior of the physical port */
struct ofp_port_mod {
    struct ofp_header header;
    big_uint32_t port_no;
    big_uint8_t pad[4];
    big_uint8_t hw_addr[OFP_ETH_ALEN]; /* The hardware address is not
                                      configurable.  This is used to
                                      sanity-check the request, so it must
                                      be the same as returned in an
                                      ofp_port struct. */
    big_uint8_t pad2[2];        /* Pad to 64 bits. */
    big_uint32_t config;        /* Bitmap of OFPPC_* flags. */
    big_uint32_t mask;          /* Bitmap of OFPPC_* flags to be changed. */

    /* Port mod property list - 0 or more properties */
    struct ofp_port_mod_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_port_mod) == 32);

/* ## -------------------------- ## */
/* ## OpenFlow Extensible Match. ## */
/* ## -------------------------- ## */

/* The match type indicates the match structure (set of fields that compose the
 * match) in use. The match type is placed in the type field at the beginning
 * of all match structures. The "OpenFlow Extensible Match" type corresponds
 * to OXM TLV format described below and must be supported by all OpenFlow
 * switches. Extensions that define other match types may be published on the
 * ONF wiki. Support for extensions is optional.
 */
enum ofp_match_type {
    OFPMT_STANDARD = 0,       /* Deprecated. */
    OFPMT_OXM      = 1,       /* OpenFlow Extensible Match */
};

/* Fields to match against flows */
struct ofp_match {
    big_uint16_t type;             /* One of OFPMT_* */
    big_uint16_t length;           /* Length of ofp_match (excluding padding) */
    /* Followed by:
     *   - Exactly (length - 4) (possibly 0) bytes containing OXM TLVs, then
     *   - Exactly ((length + 7)/8*8 - length) (between 0 and 7) bytes of
     *     all-zero bytes
     * In summary, ofp_match is padded as needed, to make its overall size
     * a multiple of 8, to preserve alignment in structures using it.
     */
    big_uint8_t oxm_fields[0];     /* 0 or more OXM match fields */
    big_uint8_t pad[4];            /* Zero bytes - see above for sizing */
};
OFP_ASSERT(sizeof(struct ofp_match) == 8);

/* Components of a OXM TLV header.
 * Those macros are not valid for the experimenter class, macros for the
 * experimenter class will depend on the experimenter header used. */
#define OXM_HEADER__(CLASS, FIELD, HASMASK, LENGTH) \
    (((CLASS) << 16) | ((FIELD) << 9) | ((HASMASK) << 8) | (LENGTH))
#define OXM_HEADER(CLASS, FIELD, LENGTH) \
    OXM_HEADER__(CLASS, FIELD, 0, LENGTH)
#define OXM_HEADER_W(CLASS, FIELD, LENGTH) \
    OXM_HEADER__(CLASS, FIELD, 1, (LENGTH) * 2)
#define OXM_CLASS(HEADER) ((HEADER) >> 16)
#define OXM_FIELD(HEADER) (((HEADER) >> 9) & 0x7f)
#define OXM_TYPE(HEADER) (((HEADER) >> 9) & 0x7fffff)
#define OXM_HASMASK(HEADER) (((HEADER) >> 8) & 1)
#define OXM_LENGTH(HEADER) ((HEADER) & 0xff)

#define OXM_MAKE_WILD_HEADER(HEADER) \
        OXM_HEADER_W(OXM_CLASS(HEADER), OXM_FIELD(HEADER), OXM_LENGTH(HEADER))

/* OXM Class IDs.
 * The high order bit differentiate reserved classes from member classes.
 * Classes 0x0000 to 0x7FFF are member classes, allocated by ONF.
 * Classes 0x8000 to 0xFFFE are reserved classes, reserved for standardisation.
 */
enum ofp_oxm_class {
    OFPXMC_NXM_0          = 0x0000,    /* Backward compatibility with NXM */
    OFPXMC_NXM_1          = 0x0001,    /* Backward compatibility with NXM */
    OFPXMC_OPENFLOW_BASIC = 0x8000,    /* Basic class for OpenFlow */
    OFPXMC_EXPERIMENTER   = 0xFFFF,    /* Experimenter class */
};

/* OXM Flow match field types for OpenFlow basic class. */
enum oxm_ofb_match_fields {
    OFPXMT_OFB_IN_PORT        = 0,  /* Switch input port. */
    OFPXMT_OFB_IN_PHY_PORT    = 1,  /* Switch physical input port. */
    OFPXMT_OFB_METADATA       = 2,  /* Metadata passed between tables. */
    OFPXMT_OFB_ETH_DST        = 3,  /* Ethernet destination address. */
    OFPXMT_OFB_ETH_SRC        = 4,  /* Ethernet source address. */
    OFPXMT_OFB_ETH_TYPE       = 5,  /* Ethernet frame type. */
    OFPXMT_OFB_VLAN_VID       = 6,  /* VLAN id. */
    OFPXMT_OFB_VLAN_PCP       = 7,  /* VLAN priority. */
    OFPXMT_OFB_IP_DSCP        = 8,  /* IP DSCP (6 bits in ToS field). */
    OFPXMT_OFB_IP_ECN         = 9,  /* IP ECN (2 bits in ToS field). */
    OFPXMT_OFB_IP_PROTO       = 10, /* IP protocol. */
    OFPXMT_OFB_IPV4_SRC       = 11, /* IPv4 source address. */
    OFPXMT_OFB_IPV4_DST       = 12, /* IPv4 destination address. */
    OFPXMT_OFB_TCP_SRC        = 13, /* TCP source port. */
    OFPXMT_OFB_TCP_DST        = 14, /* TCP destination port. */
    OFPXMT_OFB_UDP_SRC        = 15, /* UDP source port. */
    OFPXMT_OFB_UDP_DST        = 16, /* UDP destination port. */
    OFPXMT_OFB_SCTP_SRC       = 17, /* SCTP source port. */
    OFPXMT_OFB_SCTP_DST       = 18, /* SCTP destination port. */
    OFPXMT_OFB_ICMPV4_TYPE    = 19, /* ICMP type. */
    OFPXMT_OFB_ICMPV4_CODE    = 20, /* ICMP code. */
    OFPXMT_OFB_ARP_OP         = 21, /* ARP opcode. */
    OFPXMT_OFB_ARP_SPA        = 22, /* ARP source IPv4 address. */
    OFPXMT_OFB_ARP_TPA        = 23, /* ARP target IPv4 address. */
    OFPXMT_OFB_ARP_SHA        = 24, /* ARP source hardware address. */
    OFPXMT_OFB_ARP_THA        = 25, /* ARP target hardware address. */
    OFPXMT_OFB_IPV6_SRC       = 26, /* IPv6 source address. */
    OFPXMT_OFB_IPV6_DST       = 27, /* IPv6 destination address. */
    OFPXMT_OFB_IPV6_FLABEL    = 28, /* IPv6 Flow Label */
    OFPXMT_OFB_ICMPV6_TYPE    = 29, /* ICMPv6 type. */
    OFPXMT_OFB_ICMPV6_CODE    = 30, /* ICMPv6 code. */
    OFPXMT_OFB_IPV6_ND_TARGET = 31, /* Target address for ND. */
    OFPXMT_OFB_IPV6_ND_SLL    = 32, /* Source link-layer for ND. */
    OFPXMT_OFB_IPV6_ND_TLL    = 33, /* Target link-layer for ND. */
    OFPXMT_OFB_MPLS_LABEL     = 34, /* MPLS label. */
    OFPXMT_OFB_MPLS_TC        = 35, /* MPLS TC. */
    OFPXMT_OFP_MPLS_BOS       = 36, /* MPLS BoS bit. */
    OFPXMT_OFB_PBB_ISID       = 37, /* PBB I-SID. */
    OFPXMT_OFB_TUNNEL_ID      = 38, /* Logical Port Metadata. */
    OFPXMT_OFB_IPV6_EXTHDR    = 39, /* IPv6 Extension Header pseudo-field */
    OFPXMT_OFB_PBB_UCA        = 41, /* PBB UCA header field. */
};

#define OFPXMT_OFB_ALL    ((UINT64_C(1) << 42) - 1)

/* OpenFlow port on which the packet was received.
 * May be a physical port, a logical port, or the reserved port OFPP_LOCAL
 *
 * Prereqs: None.
 *
 * Format: 32-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_IN_PORT    OXM_HEADER  (0x8000, OFPXMT_OFB_IN_PORT, 4)

/* Physical port on which the packet was received.
 *
 * Consider a packet received on a tunnel interface defined over a link
 * aggregation group (LAG) with two physical port members.  If the tunnel
 * interface is the logical port bound to OpenFlow.  In this case,
 * OFPXMT_OF_IN_PORT is the tunnel's port number and OFPXMT_OF_IN_PHY_PORT is
 * the physical port number of the LAG on which the tunnel is configured.
 *
 * When a packet is received directly on a physical port and not processed by a
 * logical port, OFPXMT_OF_IN_PORT and OFPXMT_OF_IN_PHY_PORT have the same
 * value.
 *
 * This field is usually not available in a regular match and only available
 * in ofp_packet_in messages when it's different from OXM_OF_IN_PORT.
 *
 * Prereqs: OXM_OF_IN_PORT must be present.
 *
 * Format: 32-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_IN_PHY_PORT OXM_HEADER  (0x8000, OFPXMT_OFB_IN_PHY_PORT, 4)

/* Table metadata.
 *
 * Prereqs: None.
 *
 * Format: 64-bit integer in network byte order.
 *
 * Masking: Arbitrary masks.
 */
#define OXM_OF_METADATA   OXM_HEADER  (0x8000, OFPXMT_OFB_METADATA, 8)
#define OXM_OF_METADATA_W OXM_HEADER_W(0x8000, OFPXMT_OFB_METADATA, 8)

/* Source or destination address in Ethernet header.
 *
 * Prereqs: None.
 *
 * Format: 48-bit Ethernet MAC address.
 *
 * Masking: Arbitrary masks. */
#define OXM_OF_ETH_DST    OXM_HEADER  (0x8000, OFPXMT_OFB_ETH_DST, 6)
#define OXM_OF_ETH_DST_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_ETH_DST, 6)
#define OXM_OF_ETH_SRC    OXM_HEADER  (0x8000, OFPXMT_OFB_ETH_SRC, 6)
#define OXM_OF_ETH_SRC_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_ETH_SRC, 6)

/* Packet's Ethernet type.
 *
 * Prereqs: None.
 *
 * Format: 16-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_ETH_TYPE   OXM_HEADER  (0x8000, OFPXMT_OFB_ETH_TYPE, 2)

/* The VLAN id is 12-bits, so we can use the entire 16 bits to indicate
 * special conditions.
 */
enum ofp_vlan_id {
    OFPVID_PRESENT = 0x1000, /* Bit that indicate that a VLAN id is set */
    OFPVID_NONE    = 0x0000, /* No VLAN id was set. */
};
/* Define for compatibility */
#define OFP_VLAN_NONE      OFPVID_NONE

/* 802.1Q VID.
 *
 * For a packet with an 802.1Q header, this is the VLAN-ID (VID) from the
 * outermost tag, with the CFI bit forced to 1. For a packet with no 802.1Q
 * header, this has value OFPVID_NONE.
 *
 * Prereqs: None.
 *
 * Format: 16-bit integer in network byte order with bit 13 indicating
 * presence of VLAN header and 3 most-significant bits forced to 0.
 * Only the lower 13 bits have meaning.
 *
 * Masking: Arbitrary masks.
 *
 * This field can be used in various ways:
 *
 *   - If it is not constrained at all, the nx_match matches packets without
 *     an 802.1Q header or with an 802.1Q header that has any VID value.
 *
 *   - Testing for an exact match with 0x0 matches only packets without
 *     an 802.1Q header.
 *
 *   - Testing for an exact match with a VID value with CFI=1 matches packets
 *     that have an 802.1Q header with a specified VID.
 *
 *   - Testing for an exact match with a nonzero VID value with CFI=0 does
 *     not make sense.  The switch may reject this combination.
 *
 *   - Testing with nxm_value=0, nxm_mask=0x0fff matches packets with no 802.1Q
 *     header or with an 802.1Q header with a VID of 0.
 *
 *   - Testing with nxm_value=0x1000, nxm_mask=0x1000 matches packets with
 *     an 802.1Q header that has any VID value.
 */
#define OXM_OF_VLAN_VID   OXM_HEADER  (0x8000, OFPXMT_OFB_VLAN_VID, 2)
#define OXM_OF_VLAN_VID_W OXM_HEADER_W(0x8000, OFPXMT_OFB_VLAN_VID, 2)

/* 802.1Q PCP.
 *
 * For a packet with an 802.1Q header, this is the VLAN-PCP from the
 * outermost tag.  For a packet with no 802.1Q header, this has value
 * 0.
 *
 * Prereqs: OXM_OF_VLAN_VID must be different from OFPVID_NONE.
 *
 * Format: 8-bit integer with 5 most-significant bits forced to 0.
 * Only the lower 3 bits have meaning.
 *
 * Masking: Not maskable.
 */
#define OXM_OF_VLAN_PCP   OXM_HEADER  (0x8000, OFPXMT_OFB_VLAN_PCP, 1)

/* The Diff Serv Code Point (DSCP) bits of the IP header.
 * Part of the IPv4 ToS field or the IPv6 Traffic Class field.
 *
 * Prereqs: OXM_OF_ETH_TYPE must be either 0x0800 or 0x86dd.
 *
 * Format: 8-bit integer with 2 most-significant bits forced to 0.
 * Only the lower 6 bits have meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_IP_DSCP     OXM_HEADER  (0x8000, OFPXMT_OFB_IP_DSCP, 1)

/* The ECN bits of the IP header.
 * Part of the IPv4 ToS field or the IPv6 Traffic Class field.
 *
 * Prereqs: OXM_OF_ETH_TYPE must be either 0x0800 or 0x86dd.
 *
 * Format: 8-bit integer with 6 most-significant bits forced to 0.
 * Only the lower 2 bits have meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_IP_ECN     OXM_HEADER  (0x8000, OFPXMT_OFB_IP_ECN, 1)

/* The "protocol" byte in the IP header.
 *
 * Prereqs: OXM_OF_ETH_TYPE must be either 0x0800 or 0x86dd.
 *
 * Format: 8-bit integer.
 *
 * Masking: Not maskable. */
#define OXM_OF_IP_PROTO   OXM_HEADER  (0x8000, OFPXMT_OFB_IP_PROTO, 1)

/* The source or destination address in the IP header.
 *
 * Prereqs: OXM_OF_ETH_TYPE must match 0x0800 exactly.
 *
 * Format: 32-bit integer in network byte order.
 *
 * Masking: Arbitrary masks.
 */
#define OXM_OF_IPV4_SRC     OXM_HEADER  (0x8000, OFPXMT_OFB_IPV4_SRC, 4)
#define OXM_OF_IPV4_SRC_W   OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV4_SRC, 4)
#define OXM_OF_IPV4_DST     OXM_HEADER  (0x8000, OFPXMT_OFB_IPV4_DST, 4)
#define OXM_OF_IPV4_DST_W   OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV4_DST, 4)

/* The source or destination port in the TCP header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must be either 0x0800 or 0x86dd.
 *   OXM_OF_IP_PROTO must match 6 exactly.
 *
 * Format: 16-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_TCP_SRC    OXM_HEADER  (0x8000, OFPXMT_OFB_TCP_SRC, 2)
#define OXM_OF_TCP_DST    OXM_HEADER  (0x8000, OFPXMT_OFB_TCP_DST, 2)

/* The source or destination port in the UDP header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match either 0x0800 or 0x86dd.
 *   OXM_OF_IP_PROTO must match 17 exactly.
 *
 * Format: 16-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_UDP_SRC    OXM_HEADER  (0x8000, OFPXMT_OFB_UDP_SRC, 2)
#define OXM_OF_UDP_DST    OXM_HEADER  (0x8000, OFPXMT_OFB_UDP_DST, 2)

/* The source or destination port in the SCTP header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match either 0x0800 or 0x86dd.
 *   OXM_OF_IP_PROTO must match 132 exactly.
 *
 * Format: 16-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_SCTP_SRC   OXM_HEADER  (0x8000, OFPXMT_OFB_SCTP_SRC, 2)
#define OXM_OF_SCTP_DST   OXM_HEADER  (0x8000, OFPXMT_OFB_SCTP_DST, 2)

/* The type or code in the ICMP header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x0800 exactly.
 *   OXM_OF_IP_PROTO must match 1 exactly.
 *
 * Format: 8-bit integer.
 *
 * Masking: Not maskable. */
#define OXM_OF_ICMPV4_TYPE  OXM_HEADER  (0x8000, OFPXMT_OFB_ICMPV4_TYPE, 1)
#define OXM_OF_ICMPV4_CODE  OXM_HEADER  (0x8000, OFPXMT_OFB_ICMPV4_CODE, 1)

/* ARP opcode.
 *
 * For an Ethernet+IP ARP packet, the opcode in the ARP header.  Always 0
 * otherwise.
 *
 * Prereqs: OXM_OF_ETH_TYPE must match 0x0806 exactly.
 *
 * Format: 16-bit integer in network byte order.
 *
 * Masking: Not maskable. */
#define OXM_OF_ARP_OP     OXM_HEADER  (0x8000, OFPXMT_OFB_ARP_OP, 2)

/* For an Ethernet+IP ARP packet, the source or target protocol address
 * in the ARP header.  Always 0 otherwise.
 *
 * Prereqs: OXM_OF_ETH_TYPE must match 0x0806 exactly.
 *
 * Format: 32-bit integer in network byte order.
 *
 * Masking: Arbitrary masks.
 */
#define OXM_OF_ARP_SPA    OXM_HEADER  (0x8000, OFPXMT_OFB_ARP_SPA, 4)
#define OXM_OF_ARP_SPA_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_ARP_SPA, 4)
#define OXM_OF_ARP_TPA    OXM_HEADER  (0x8000, OFPXMT_OFB_ARP_TPA, 4)
#define OXM_OF_ARP_TPA_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_ARP_TPA, 4)

/* For an Ethernet+IP ARP packet, the source or target hardware address
 * in the ARP header.  Always 0 otherwise.
 *
 * Prereqs: OXM_OF_ETH_TYPE must match 0x0806 exactly.
 *
 * Format: 48-bit Ethernet MAC address.
 *
 * Masking: Not maskable. */
#define OXM_OF_ARP_SHA    OXM_HEADER  (0x8000, OFPXMT_OFB_ARP_SHA, 6)
#define OXM_OF_ARP_THA    OXM_HEADER  (0x8000, OFPXMT_OFB_ARP_THA, 6)

/* The source or destination address in the IPv6 header.
 *
 * Prereqs: OXM_OF_ETH_TYPE must match 0x86dd exactly.
 *
 * Format: 128-bit IPv6 address.
 *
 * Masking: Arbitrary masks.
 */
#define OXM_OF_IPV6_SRC    OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_SRC, 16)
#define OXM_OF_IPV6_SRC_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV6_SRC, 16)
#define OXM_OF_IPV6_DST    OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_DST, 16)
#define OXM_OF_IPV6_DST_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV6_DST, 16)

/* The IPv6 Flow Label
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly
 *
 * Format: 32-bit integer with 12 most-significant bits forced to 0.
 * Only the lower 20 bits have meaning.
 *
 * Masking: Arbitrary masks.
 */
#define OXM_OF_IPV6_FLABEL   OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_FLABEL, 4)
#define OXM_OF_IPV6_FLABEL_W OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV6_FLABEL, 4)

/* The type or code in the ICMPv6 header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly.
 *   OXM_OF_IP_PROTO must match 58 exactly.
 *
 * Format: 8-bit integer.
 *
 * Masking: Not maskable. */
#define OXM_OF_ICMPV6_TYPE OXM_HEADER  (0x8000, OFPXMT_OFB_ICMPV6_TYPE, 1)
#define OXM_OF_ICMPV6_CODE OXM_HEADER  (0x8000, OFPXMT_OFB_ICMPV6_CODE, 1)

/* The target address in an IPv6 Neighbor Discovery message.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly.
 *   OXM_OF_IP_PROTO must match 58 exactly.
 *   OXM_OF_ICMPV6_TYPE must be either 135 or 136.
 *
 * Format: 128-bit IPv6 address.
 *
 * Masking: Not maskable. */
#define OXM_OF_IPV6_ND_TARGET OXM_HEADER (0x8000, OFPXMT_OFB_IPV6_ND_TARGET, 16)

/* The source link-layer address option in an IPv6 Neighbor Discovery
 * message.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly.
 *   OXM_OF_IP_PROTO must match 58 exactly.
 *   OXM_OF_ICMPV6_TYPE must be exactly 135.
 *
 * Format: 48-bit Ethernet MAC address.
 *
 * Masking: Not maskable. */
#define OXM_OF_IPV6_ND_SLL  OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_ND_SLL, 6)

/* The target link-layer address option in an IPv6 Neighbor Discovery
 * message.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly.
 *   OXM_OF_IP_PROTO must match 58 exactly.
 *   OXM_OF_ICMPV6_TYPE must be exactly 136.
 *
 * Format: 48-bit Ethernet MAC address.
 *
 * Masking: Not maskable. */
#define OXM_OF_IPV6_ND_TLL  OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_ND_TLL, 6)

/* The LABEL in the first MPLS shim header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x8847 or 0x8848 exactly.
 *
 * Format: 32-bit integer in network byte order with 12 most-significant
 * bits forced to 0. Only the lower 20 bits have meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_MPLS_LABEL  OXM_HEADER  (0x8000, OFPXMT_OFB_MPLS_LABEL, 4)

/* The TC in the first MPLS shim header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x8847 or 0x8848 exactly.
 *
 * Format: 8-bit integer with 5 most-significant bits forced to 0.
 * Only the lower 3 bits have meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_MPLS_TC     OXM_HEADER  (0x8000, OFPXMT_OFB_MPLS_TC, 1)

/* The BoS bit in the first MPLS shim header.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x8847 or 0x8848 exactly.
 *
 * Format: 8-bit integer with 7 most-significant bits forced to 0.
 * Only the lowest bit have a meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_MPLS_BOS     OXM_HEADER  (0x8000, OFPXMT_OFB_MPLS_BOS, 1)

/* IEEE 802.1ah I-SID.
 *
 * For a packet with a PBB header, this is the I-SID from the
 * outermost service tag.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x88E7 exactly.
 *
 * Format: 24-bit integer in network byte order.
 *
 * Masking: Arbitrary masks. */
#define OXM_OF_PBB_ISID   OXM_HEADER  (0x8000, OFPXMT_OFB_PBB_ISID, 3)
#define OXM_OF_PBB_ISID_W OXM_HEADER_W(0x8000, OFPXMT_OFB_PBB_ISID, 3)

/* Logical Port Metadata.
 *
 * Metadata associated with a logical port.
 * If the logical port performs encapsulation and decapsulation, this
 * is the demultiplexing field from the encapsulation header.
 * For example, for a packet received via GRE tunnel including a (32-bit) key,
 * the key is stored in the low 32-bits and the high bits are zeroed.
 * For a MPLS logical port, the low 20 bits represent the MPLS Label.
 * For a VxLAN logical port, the low 24 bits represent the VNI.
 * If the packet is not received through a logical port, the value is 0.
 *
 * Prereqs: None.
 *
 * Format: 64-bit integer in network byte order.
 *
 * Masking: Arbitrary masks. */
#define OXM_OF_TUNNEL_ID    OXM_HEADER  (0x8000, OFPXMT_OFB_TUNNEL_ID, 8)
#define OXM_OF_TUNNEL_ID_W  OXM_HEADER_W(0x8000, OFPXMT_OFB_TUNNEL_ID, 8)

/* The IPv6 Extension Header pseudo-field.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x86dd exactly
 *
 * Format: 16-bit integer with 7 most-significant bits forced to 0.
 * Only the lower 9 bits have meaning.
 *
 * Masking: Maskable. */
#define OXM_OF_IPV6_EXTHDR   OXM_HEADER  (0x8000, OFPXMT_OFB_IPV6_EXTHDR, 2)
#define OXM_OF_IPV6_EXTHDR_W OXM_HEADER_W(0x8000, OFPXMT_OFB_IPV6_EXTHDR, 2)

/* Bit definitions for IPv6 Extension Header pseudo-field. */
enum ofp_ipv6exthdr_flags {      
    OFPIEH_NONEXT = 1 << 0,     /* "No next header" encountered. */
    OFPIEH_ESP    = 1 << 1,     /* Encrypted Sec Payload header present. */
    OFPIEH_AUTH   = 1 << 2,     /* Authentication header present. */
    OFPIEH_DEST   = 1 << 3,     /* 1 or 2 dest headers present. */
    OFPIEH_FRAG   = 1 << 4,     /* Fragment header present. */
    OFPIEH_ROUTER = 1 << 5,     /* Router header present. */
    OFPIEH_HOP    = 1 << 6,     /* Hop-by-hop header present. */
    OFPIEH_UNREP  = 1 << 7,     /* Unexpected repeats encountered. */
    OFPIEH_UNSEQ  = 1 << 8,     /* Unexpected sequencing encountered. */
};

/* IEEE 802.1ah UCA.
 *
 * For a packet with a PBB header, this is the UCA (Use Customer Address)
 * from the outermost service tag.
 *
 * Prereqs:
 *   OXM_OF_ETH_TYPE must match 0x88E7 exactly.
 *
 * Format: 8-bit integer with 7 most-significant bits forced to 0.
 * Only the lower 1 bit has meaning.
 *
 * Masking: Not maskable. */
#define OXM_OF_PBB_UCA    OXM_HEADER  (0x8000, OFPXMT_OFB_PBB_UCA, 1)

/* Header for OXM experimenter match fields.
 * The experimenter class should not use OXM_HEADER() macros for defining
 * fields due to this extra header. */
struct ofp_oxm_experimenter_header {
    big_uint32_t oxm_header;        /* oxm_class = OFPXMC_EXPERIMENTER */
    big_uint32_t experimenter;      /* Experimenter ID. */
};
OFP_ASSERT(sizeof(struct ofp_oxm_experimenter_header) == 8);

/* ## ----------------- ## */
/* ## OpenFlow Actions. ## */
/* ## ----------------- ## */

enum ofp_action_type {
    OFPAT_OUTPUT       = 0,  /* Output to switch port. */
    OFPAT_COPY_TTL_OUT = 11, /* Copy TTL "outwards" -- from next-to-outermost
                                to outermost */
    OFPAT_COPY_TTL_IN  = 12, /* Copy TTL "inwards" -- from outermost to
                               next-to-outermost */
    OFPAT_SET_MPLS_TTL = 15, /* MPLS TTL */
    OFPAT_DEC_MPLS_TTL = 16, /* Decrement MPLS TTL */

    OFPAT_PUSH_VLAN    = 17, /* Push a new VLAN tag */
    OFPAT_POP_VLAN     = 18, /* Pop the outer VLAN tag */
    OFPAT_PUSH_MPLS    = 19, /* Push a new MPLS tag */
    OFPAT_POP_MPLS     = 20, /* Pop the outer MPLS tag */
    OFPAT_SET_QUEUE    = 21, /* Set queue id when outputting to a port */
    OFPAT_GROUP        = 22, /* Apply group. */
    OFPAT_SET_NW_TTL   = 23, /* IP TTL. */
    OFPAT_DEC_NW_TTL   = 24, /* Decrement IP TTL. */
    OFPAT_SET_FIELD    = 25, /* Set a header field using OXM TLV format. */
    OFPAT_PUSH_PBB     = 26, /* Push a new PBB service tag (I-TAG) */
    OFPAT_POP_PBB      = 27, /* Pop the outer PBB service tag (I-TAG) */
    OFPAT_EXPERIMENTER = 0xffff
};

/* Action header that is common to all actions.  The length includes the
 * header and any padding used to make the action 64-bit aligned.
 * NB: The length of an action *must* always be a multiple of eight. */
struct ofp_action_header {
    big_uint16_t type;                  /* One of OFPAT_*. */
    big_uint16_t len;                   /* Length of this struct in bytes. */
};
OFP_ASSERT(sizeof(struct ofp_action_header) == 4);

enum ofp_controller_max_len {
	OFPCML_MAX       = 0xffe5, /* maximum max_len value which can be used
	                              to request a specific byte length. */
	OFPCML_NO_BUFFER = 0xffff  /* indicates that no buffering should be
	                              applied and the whole packet is to be
	                              sent to the controller. */
};

/* Action structure for OFPAT_OUTPUT, which sends packets out 'port'.
 * When the 'port' is the OFPP_CONTROLLER, 'max_len' indicates the max
 * number of bytes to send.  A 'max_len' of zero means no bytes of the
 * packet should be sent. A 'max_len' of OFPCML_NO_BUFFER means that
 * the packet is not buffered and the complete packet is to be sent to
 * the controller. */
struct ofp_action_output {
    big_uint16_t type;                  /* OFPAT_OUTPUT. */
    big_uint16_t len;                   /* Length is 16. */
    big_uint32_t port;                  /* Output port. */
    big_uint16_t max_len;               /* Max length to send to controller. */
    big_uint8_t pad[6];                 /* Pad to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_action_output) == 16);

/* Action structure for OFPAT_COPY_TTL_OUT, OFPAT_COPY_TTL_IN,
 * OFPAT_DEC_MPLS_TTL, OFPAT_DEC_NW_TTL, OFPAT_POP_VLAN and OFPAT_POP_PBB. */
struct ofp_action_generic {
    big_uint16_t type;                  /* One of OFPAT_*. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint8_t pad[4];                 /* Pad to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_action_generic) == 8);

/* Action structure for OFPAT_SET_MPLS_TTL. */
struct ofp_action_mpls_ttl {
    big_uint16_t type;                  /* OFPAT_SET_MPLS_TTL. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint8_t mpls_ttl;               /* MPLS TTL */
    big_uint8_t pad[3];
};
OFP_ASSERT(sizeof(struct ofp_action_mpls_ttl) == 8);

/* Action structure for OFPAT_PUSH_VLAN/MPLS/PBB. */
struct ofp_action_push {
    big_uint16_t type;                  /* OFPAT_PUSH_VLAN/MPLS/PBB. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint16_t ethertype;             /* Ethertype */
    big_uint8_t pad[2];
};
OFP_ASSERT(sizeof(struct ofp_action_push) == 8);

/* Action structure for OFPAT_POP_MPLS. */
struct ofp_action_pop_mpls {
    big_uint16_t type;                  /* OFPAT_POP_MPLS. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint16_t ethertype;             /* Ethertype */
    big_uint8_t pad[2];
};
OFP_ASSERT(sizeof(struct ofp_action_pop_mpls) == 8);

/* Action structure for OFPAT_SET_QUEUE. */
struct ofp_action_set_queue {
    big_uint16_t type;            /* OFPAT_SET_QUEUE. */
    big_uint16_t len;             /* Len is 8. */
    big_uint32_t queue_id;        /* Queue id for the packets. */
};
OFP_ASSERT(sizeof(struct ofp_action_set_queue) == 8);

/* Action structure for OFPAT_GROUP. */
struct ofp_action_group {
    big_uint16_t type;                  /* OFPAT_GROUP. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint32_t group_id;              /* Group identifier. */
};
OFP_ASSERT(sizeof(struct ofp_action_group) == 8);

/* Action structure for OFPAT_SET_NW_TTL. */
struct ofp_action_nw_ttl {
    big_uint16_t type;                  /* OFPAT_SET_NW_TTL. */
    big_uint16_t len;                   /* Length is 8. */
    big_uint8_t nw_ttl;                 /* IP TTL */
    big_uint8_t pad[3];
};
OFP_ASSERT(sizeof(struct ofp_action_nw_ttl) == 8);

/* Action structure for OFPAT_SET_FIELD. */
struct ofp_action_set_field {
    big_uint16_t type;                  /* OFPAT_SET_FIELD. */
    big_uint16_t len;                   /* Length is padded to 64 bits. */
    /* Followed by:
     *   - Exactly (4 + oxm_length) bytes containing a single OXM TLV, then
     *   - Exactly ((8 + oxm_length) + 7)/8*8 - (8 + oxm_length)
     *     (between 0 and 7) bytes of all-zero bytes
     */
    big_uint8_t field[4];               /* OXM TLV - Make compiler happy */
};
OFP_ASSERT(sizeof(struct ofp_action_set_field) == 8);

/* Action header for OFPAT_EXPERIMENTER.
 * The rest of the body is experimenter-defined. */
struct ofp_action_experimenter_header {
    big_uint16_t type;                  /* OFPAT_EXPERIMENTER. */
    big_uint16_t len;                   /* Length is a multiple of 8. */
    big_uint32_t experimenter;          /* Experimenter ID. */
};
OFP_ASSERT(sizeof(struct ofp_action_experimenter_header) == 8);

/* ## ---------------------- ## */
/* ## OpenFlow Instructions. ## */
/* ## ---------------------- ## */

enum ofp_instruction_type {
    OFPIT_GOTO_TABLE = 1,       /* Setup the next table in the lookup
                                   pipeline */
    OFPIT_WRITE_METADATA = 2,   /* Setup the metadata field for use later in
                                   pipeline */
    OFPIT_WRITE_ACTIONS = 3,    /* Write the action(s) onto the datapath action
                                   set */
    OFPIT_APPLY_ACTIONS = 4,    /* Applies the action(s) immediately */
    OFPIT_CLEAR_ACTIONS = 5,    /* Clears all actions from the datapath
                                   action set */
    OFPIT_METER = 6,            /* Apply meter (rate limiter) */

    OFPIT_EXPERIMENTER = 0xFFFF  /* Experimenter instruction */
};

/* Instruction header that is common to all instructions.  The length includes
 * the header and any padding used to make the instruction 64-bit aligned.
 * NB: The length of an instruction *must* always be a multiple of eight. */
struct ofp_instruction_header {
    big_uint16_t type;                /* One of OFPIT_*. */
    big_uint16_t len;                 /* Length of this struct in bytes. */
};
OFP_ASSERT(sizeof(struct ofp_instruction_header) == 4);

/* Instruction structure for OFPIT_GOTO_TABLE */
struct ofp_instruction_goto_table {
    big_uint16_t type;                /* OFPIT_GOTO_TABLE */
    big_uint16_t len;                 /* Length is 8. */
    big_uint8_t table_id;             /* Set next table in the lookup pipeline */
    big_uint8_t pad[3];               /* Pad to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_instruction_goto_table) == 8);

/* Instruction structure for OFPIT_WRITE_METADATA */
struct ofp_instruction_write_metadata {
    big_uint16_t type;                /* OFPIT_WRITE_METADATA */
    big_uint16_t len;                 /* Length is 24. */
    big_uint8_t pad[4];               /* Align to 64-bits */
    big_uint64_t metadata;            /* Metadata value to write */
    big_uint64_t metadata_mask;       /* Metadata write bitmask */
};
OFP_ASSERT(sizeof(struct ofp_instruction_write_metadata) == 24);

/* Instruction structure for OFPIT_WRITE/APPLY/CLEAR_ACTIONS */
struct ofp_instruction_actions {
    big_uint16_t type;              /* One of OFPIT_*_ACTIONS */
    big_uint16_t len;               /* Length is padded to 64 bits. */
    big_uint8_t pad[4];             /* Align to 64-bits */
    struct ofp_action_header actions[0];  /* 0 or more actions associated with
                                             OFPIT_WRITE_ACTIONS and
                                             OFPIT_APPLY_ACTIONS */
};
OFP_ASSERT(sizeof(struct ofp_instruction_actions) == 8);

/* Instruction structure for OFPIT_METER */
struct ofp_instruction_meter {
    big_uint16_t type;                /* OFPIT_METER */
    big_uint16_t len;                 /* Length is 8. */
    big_uint32_t meter_id;            /* Meter instance. */
};
OFP_ASSERT(sizeof(struct ofp_instruction_meter) == 8);

/* Instruction structure for experimental instructions */
struct ofp_instruction_experimenter_header {
    big_uint16_t type;              /* OFPIT_EXPERIMENTER. */
    big_uint16_t len;               /* Length is padded to 64 bits. */
    big_uint32_t experimenter;      /* Experimenter ID. */
    /* Experimenter-defined arbitrary additional data. */
};
OFP_ASSERT(sizeof(struct ofp_instruction_experimenter_header) == 8);

/* ## --------------------------- ## */
/* ## OpenFlow Flow Modification. ## */
/* ## --------------------------- ## */

enum ofp_flow_mod_command {
    OFPFC_ADD           = 0, /* New flow. */
    OFPFC_MODIFY        = 1, /* Modify all matching flows. */
    OFPFC_MODIFY_STRICT = 2, /* Modify entry strictly matching wildcards and
                                priority. */
    OFPFC_DELETE        = 3, /* Delete all matching flows. */
    OFPFC_DELETE_STRICT = 4, /* Delete entry strictly matching wildcards and
                                priority. */
};

/* Value used in "idle_timeout" and "hard_timeout" to indicate that the entry
 * is permanent. */
#define OFP_FLOW_PERMANENT 0

/* By default, choose a priority in the middle. */
#define OFP_DEFAULT_PRIORITY 0x8000

enum ofp_flow_mod_flags {
    OFPFF_SEND_FLOW_REM = 1 << 0,  /* Send flow removed message when flow
                                    * expires or is deleted. */
    OFPFF_CHECK_OVERLAP = 1 << 1,  /* Check for overlapping entries first. */
    OFPFF_RESET_COUNTS  = 1 << 2,  /* Reset flow packet and byte counts. */
    OFPFF_NO_PKT_COUNTS = 1 << 3,  /* Don't keep track of packet count. */
    OFPFF_NO_BYT_COUNTS = 1 << 4,  /* Don't keep track of byte count. */
};

/* Flow setup and teardown (controller -> datapath). */
struct ofp_flow_mod {
    struct ofp_header header;
    big_uint64_t cookie;              /* Opaque controller-issued identifier. */
    big_uint64_t cookie_mask;         /* Mask used to restrict the cookie bits
                                     that must match when the command is
                                     OFPFC_MODIFY* or OFPFC_DELETE*. A value
                                     of 0 indicates no restriction. */
    big_uint8_t table_id;             /* ID of the table to put the flow in.
                                     For OFPFC_DELETE_* commands, OFPTT_ALL
                                     can also be used to delete matching
                                     flows from all tables. */
    big_uint8_t command;              /* One of OFPFC_*. */
    big_uint16_t idle_timeout;        /* Idle time before discarding (seconds). */
    big_uint16_t hard_timeout;        /* Max time before discarding (seconds). */
    big_uint16_t priority;            /* Priority level of flow entry. */
    big_uint32_t buffer_id;           /* Buffered packet to apply to, or
                                     OFP_NO_BUFFER.
                                     Not meaningful for OFPFC_DELETE*. */
    big_uint32_t out_port;            /* For OFPFC_DELETE* commands, require
                                     matching entries to include this as an
                                     output port.  A value of OFPP_ANY
                                     indicates no restriction. */
    big_uint32_t out_group;           /* For OFPFC_DELETE* commands, require
                                     matching entries to include this as an
                                     output group.  A value of OFPG_ANY
                                     indicates no restriction. */
    big_uint16_t flags;               /* Bitmap of OFPFF_* flags. */
    big_uint16_t importance;          /* Eviction precedence (optional). */
    struct ofp_match match;       /* Fields to match. Variable size. */
    /* The variable size and padded match is always followed by instructions. */
    //struct ofp_instruction_header instructions[0];
	                          /* Instruction set - 0 or more. The length
                                     of the instruction set is inferred from
                                     the length field in the header. */
};
OFP_ASSERT(sizeof(struct ofp_flow_mod) == 56);

/* Group numbering. Groups can use any number up to OFPG_MAX. */
enum ofp_group {
    /* Last usable group number. */
    OFPG_MAX        = 0xffffff00,

    /* Fake groups. */
    OFPG_ALL        = 0xfffffffc,  /* Represents all groups for group delete
                                      commands. */
    OFPG_ANY        = 0xffffffff   /* Special wildcard: no group specified. */
};

/* Group commands */
enum ofp_group_mod_command {
    OFPGC_ADD    = 0,       /* New group. */
    OFPGC_MODIFY = 1,       /* Modify all matching groups. */
    OFPGC_DELETE = 2,       /* Delete all matching groups. */
};

/* Bucket for use in groups. */
struct ofp_bucket {
    big_uint16_t len;                   /* Length of the bucket in bytes, including
                                       this header and any padding to make it
                                       64-bit aligned. */
    big_uint16_t weight;                /* Relative weight of bucket.  Only
                                       defined for select groups. */
    big_uint32_t watch_port;            /* Port whose state affects whether this
                                       bucket is live.  Only required for fast
                                       failover groups. */
    big_uint32_t watch_group;           /* Group whose state affects whether this
                                       bucket is live.  Only required for fast
                                       failover groups. */
    big_uint8_t pad[4];
    struct ofp_action_header actions[0]; /* 0 or more actions associated with
                                            the bucket - The action list length
                                            is inferred from the length
                                            of the bucket. */
};
OFP_ASSERT(sizeof(struct ofp_bucket) == 16);

/* Group setup and teardown (controller -> datapath). */
struct ofp_group_mod {
    struct ofp_header header;
    big_uint16_t command;             /* One of OFPGC_*. */
    big_uint8_t type;                 /* One of OFPGT_*. */
    big_uint8_t pad;                  /* Pad to 64 bits. */
    big_uint32_t group_id;            /* Group identifier. */
    struct ofp_bucket buckets[0]; /* The length of the bucket array is inferred
                                     from the length field in the header. */
};
OFP_ASSERT(sizeof(struct ofp_group_mod) == 16);

/* Group types.  Values in the range [128, 255] are reserved for experimental
 * use. */
enum ofp_group_type {
    OFPGT_ALL      = 0, /* All (multicast/broadcast) group.  */
    OFPGT_SELECT   = 1, /* Select group. */
    OFPGT_INDIRECT = 2, /* Indirect group. */
    OFPGT_FF       = 3, /* Fast failover group. */
};

/* Special buffer-id to indicate 'no buffer' */
#define OFP_NO_BUFFER 0xffffffff

/* Send packet (controller -> datapath). */
struct ofp_packet_out {
    struct ofp_header header;
    big_uint32_t buffer_id;           /* ID assigned by datapath (OFP_NO_BUFFER
                                     if none). */
    big_uint32_t in_port;             /* Packet's input port or OFPP_CONTROLLER. */
    big_uint16_t actions_len;         /* Size of action array in bytes. */
    big_uint8_t pad[6];
    struct ofp_action_header actions[0]; /* Action list - 0 or more. */
    /* The variable size action list is optionally followed by packet data.
     * This data is only present and meaningful if buffer_id == -1. */
    /* big_uint8_t data[0]; */        /* Packet data.  The length is inferred
                                     from the length field in the header. */
};
OFP_ASSERT(sizeof(struct ofp_packet_out) == 24);

/* Why is this packet being sent to the controller? */
enum ofp_packet_in_reason {
    OFPR_TABLE_MISS   = 0,   /* No matching flow (table-miss flow entry). */
    OFPR_APPLY_ACTION = 1,   /* Output to controller in apply-actions. */
    OFPR_INVALID_TTL  = 2,   /* Packet has invalid TTL */
    OFPR_ACTION_SET   = 3,   /* Output to controller in action set. */
    OFPR_GROUP        = 4,   /* Output to controller in group bucket. */
    OFPR_PACKET_OUT   = 5,   /* Output to controller in packet-out. */
};

/* Packet received on port (datapath -> controller). */
struct ofp_packet_in {
    struct ofp_header header;
    big_uint32_t buffer_id;     /* ID assigned by datapath. */
    big_uint16_t total_len;     /* Full length of frame. */
    big_uint8_t reason;         /* Reason packet is being sent (one of OFPR_*) */
    big_uint8_t table_id;       /* ID of the table that was looked up */
    big_uint64_t cookie;        /* Cookie of the flow entry that was looked up. */
    struct ofp_match match; /* Packet metadata. Variable size. */
    /* The variable size and padded match is always followed by:
     *   - Exactly 2 all-zero padding bytes, then
     *   - An Ethernet frame whose length is inferred from header.length.
     * The padding bytes preceding the Ethernet frame ensure that the IP
     * header (if any) following the Ethernet header is 32-bit aligned.
     */
    //big_uint8_t pad[2];       /* Align to 64 bit + 16 bit */
    //big_uint8_t data[0];      /* Ethernet frame */
};
OFP_ASSERT(sizeof(struct ofp_packet_in) == 32);

/* Why was this flow removed? */
enum ofp_flow_removed_reason {
    OFPRR_IDLE_TIMEOUT = 0,     /* Flow idle time exceeded idle_timeout. */
    OFPRR_HARD_TIMEOUT = 1,     /* Time exceeded hard_timeout. */
    OFPRR_DELETE       = 2,     /* Evicted by a DELETE flow mod. */
    OFPRR_GROUP_DELETE = 3,     /* Group was removed. */
    OFPRR_METER_DELETE = 4,     /* Meter was removed. */
    OFPRR_EVICTION     = 5,     /* Switch eviction to free resources. */
};

/* Flow removed (datapath -> controller). */
struct ofp_flow_removed {
    struct ofp_header header;
    big_uint64_t cookie;          /* Opaque controller-issued identifier. */

    big_uint16_t priority;        /* Priority level of flow entry. */
    big_uint8_t reason;           /* One of OFPRR_*. */
    big_uint8_t table_id;         /* ID of the table */

    big_uint32_t duration_sec;    /* Time flow was alive in seconds. */
    big_uint32_t duration_nsec;   /* Time flow was alive in nanoseconds beyond
                                 duration_sec. */
    big_uint16_t idle_timeout;    /* Idle timeout from original flow mod. */
    big_uint16_t hard_timeout;    /* Hard timeout from original flow mod. */
    big_uint64_t packet_count;
    big_uint64_t byte_count;
    struct ofp_match match;   /* Description of fields. Variable size. */
};
OFP_ASSERT(sizeof(struct ofp_flow_removed) == 56);

/* Meter numbering. Flow meters can use any number up to OFPM_MAX. */
enum ofp_meter {
    /* Last usable meter. */
    OFPM_MAX        = 0xffff0000,

    /* Virtual meters. */
    OFPM_SLOWPATH   = 0xfffffffd,  /* Meter for slow datapath. */
    OFPM_CONTROLLER = 0xfffffffe,  /* Meter for controller connection. */
    OFPM_ALL        = 0xffffffff,  /* Represents all meters for stat requests
                                      commands. */
};

/* Meter band types */
enum ofp_meter_band_type {
    OFPMBT_DROP            = 1,      /* Drop packet. */
    OFPMBT_DSCP_REMARK     = 2,      /* Remark DSCP in the IP header. */
    OFPMBT_EXPERIMENTER    = 0xFFFF  /* Experimenter meter band. */
};

/* Common header for all meter bands */
struct ofp_meter_band_header {
    big_uint16_t        type;    /* One of OFPMBT_*. */
    big_uint16_t        len;     /* Length in bytes of this band. */
    big_uint32_t        rate;    /* Rate for this band. */
    big_uint32_t        burst_size; /* Size of bursts. */
};
OFP_ASSERT(sizeof(struct ofp_meter_band_header) == 12);

/* OFPMBT_DROP band - drop packets */
struct ofp_meter_band_drop {
    big_uint16_t        type;    /* OFPMBT_DROP. */
    big_uint16_t        len;     /* Length is 16. */
    big_uint32_t        rate;    /* Rate for dropping packets. */
    big_uint32_t        burst_size; /* Size of bursts. */
    big_uint8_t         pad[4];
};
OFP_ASSERT(sizeof(struct ofp_meter_band_drop) == 16);

/* OFPMBT_DSCP_REMARK band - Remark DSCP in the IP header */
struct ofp_meter_band_dscp_remark {
    big_uint16_t        type;    /* OFPMBT_DSCP_REMARK. */
    big_uint16_t        len;     /* Length is 16. */
    big_uint32_t        rate;    /* Rate for remarking packets. */
    big_uint32_t        burst_size; /* Size of bursts. */
    big_uint8_t         prec_level; /* Number of drop precedence level to add. */
    big_uint8_t         pad[3];
};
OFP_ASSERT(sizeof(struct ofp_meter_band_dscp_remark) == 16);

/* OFPMBT_EXPERIMENTER band - Experimenter type.
 * The rest of the band is experimenter-defined. */
struct ofp_meter_band_experimenter {
    big_uint16_t        type;    /* One of OFPMBT_*. */
    big_uint16_t        len;     /* Length in bytes of this band. */
    big_uint32_t        rate;    /* Rate for this band. */
    big_uint32_t        burst_size;   /* Size of bursts. */
    big_uint32_t        experimenter; /* Experimenter ID. */
};
OFP_ASSERT(sizeof(struct ofp_meter_band_experimenter) == 16);

/* Meter commands */
enum ofp_meter_mod_command {
    OFPMC_ADD,              /* New meter. */
    OFPMC_MODIFY,           /* Modify specified meter. */
    OFPMC_DELETE,           /* Delete specified meter. */
};

/* Meter configuration flags */
enum ofp_meter_flags {
    OFPMF_KBPS    = 1 << 0,     /* Rate value in kb/s (kilo-bit per second). */
    OFPMF_PKTPS   = 1 << 1,     /* Rate value in packet/sec. */
    OFPMF_BURST   = 1 << 2,     /* Do burst size. */
    OFPMF_STATS   = 1 << 3,     /* Collect statistics. */
};

/* Meter configuration. OFPT_METER_MOD. */
struct ofp_meter_mod {
    struct ofp_header	header;
    big_uint16_t            command;        /* One of OFPMC_*. */
    big_uint16_t            flags;          /* Bitmap of OFPMF_* flags. */
    big_uint32_t            meter_id;       /* Meter instance. */
    struct ofp_meter_band_header bands[0]; /* The band list length is
                                           inferred from the length field
                                           in the header. */
};
OFP_ASSERT(sizeof(struct ofp_meter_mod) == 16);

/* Values for 'type' in ofp_error_message.  These values are immutable: they
 * will not change in future versions of the protocol (although new values may
 * be added). */
enum ofp_error_type {
    OFPET_HELLO_FAILED         = 0,  /* Hello protocol failed. */
    OFPET_BAD_REQUEST          = 1,  /* Request was not understood. */
    OFPET_BAD_ACTION           = 2,  /* Error in action description. */
    OFPET_BAD_INSTRUCTION      = 3,  /* Error in instruction list. */
    OFPET_BAD_MATCH            = 4,  /* Error in match. */
    OFPET_FLOW_MOD_FAILED      = 5,  /* Problem modifying flow entry. */
    OFPET_GROUP_MOD_FAILED     = 6,  /* Problem modifying group entry. */
    OFPET_PORT_MOD_FAILED      = 7,  /* Port mod request failed. */
    OFPET_TABLE_MOD_FAILED     = 8,  /* Table mod request failed. */
    OFPET_QUEUE_OP_FAILED      = 9,  /* Queue operation failed. */
    OFPET_SWITCH_CONFIG_FAILED = 10, /* Switch config request failed. */
    OFPET_ROLE_REQUEST_FAILED  = 11, /* Controller Role request failed. */
    OFPET_METER_MOD_FAILED     = 12, /* Error in meter. */
    OFPET_TABLE_FEATURES_FAILED = 13, /* Setting table features failed. */
    OFPET_BAD_PROPERTY         = 14, /* Some property is invalid. */
    OFPET_ASYNC_CONFIG_FAILED  = 15, /* Asynchronous config request failed. */
    OFPET_FLOW_MONITOR_FAILED  = 16, /* Setting flow monitor failed. */
    OFPET_BUNDLE_FAILED        = 17, /* Bundle operation failed. */
    OFPET_EXPERIMENTER = 0xffff      /* Experimenter error messages. */
};

/* ofp_error_msg 'code' values for OFPET_HELLO_FAILED.  'data' contains an
 * ASCII text string that may give failure details. */
enum ofp_hello_failed_code {
    OFPHFC_INCOMPATIBLE = 0,    /* No compatible version. */
    OFPHFC_EPERM        = 1,    /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_BAD_REQUEST.  'data' contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_request_code {
    OFPBRC_BAD_VERSION      = 0,  /* ofp_header.version not supported. */
    OFPBRC_BAD_TYPE         = 1,  /* ofp_header.type not supported. */
    OFPBRC_BAD_MULTIPART    = 2,  /* ofp_multipart_request.type not supported. */
    OFPBRC_BAD_EXPERIMENTER = 3,  /* Experimenter id not supported
                                   * (in ofp_experimenter_header or
                                   * ofp_multipart_request or
                                   * ofp_multipart_reply). */
    OFPBRC_BAD_EXP_TYPE     = 4,  /* Experimenter type not supported. */
    OFPBRC_EPERM            = 5,  /* Permissions error. */
    OFPBRC_BAD_LEN          = 6,  /* Wrong request length for type. */
    OFPBRC_BUFFER_EMPTY     = 7,  /* Specified buffer has already been used. */
    OFPBRC_BUFFER_UNKNOWN   = 8,  /* Specified buffer does not exist. */
    OFPBRC_BAD_TABLE_ID     = 9,  /* Specified table-id invalid or does not
                                   * exist. */
    OFPBRC_IS_SLAVE         = 10, /* Denied because controller is slave. */
    OFPBRC_BAD_PORT         = 11, /* Invalid port. */
    OFPBRC_BAD_PACKET       = 12, /* Invalid packet in packet-out. */
    OFPBRC_MULTIPART_BUFFER_OVERFLOW    = 13, /* ofp_multipart_request
                                     overflowed the assigned buffer. */
    OFPBRC_MULTIPART_REQUEST_TIMEOUT = 14, /* Timeout during multipart request. */
    OFPBRC_MULTIPART_REPLY_TIMEOUT = 15, /* Timeout during multipart reply. */
};

/* ofp_error_msg 'code' values for OFPET_BAD_ACTION.  'data' contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_action_code {
    OFPBAC_BAD_TYPE           = 0,  /* Unknown or unsupported action type. */
    OFPBAC_BAD_LEN            = 1,  /* Length problem in actions. */
    OFPBAC_BAD_EXPERIMENTER   = 2,  /* Unknown experimenter id specified. */
    OFPBAC_BAD_EXP_TYPE       = 3,  /* Unknown action for experimenter id. */
    OFPBAC_BAD_OUT_PORT       = 4,  /* Problem validating output port. */
    OFPBAC_BAD_ARGUMENT       = 5,  /* Bad action argument. */
    OFPBAC_EPERM              = 6,  /* Permissions error. */
    OFPBAC_TOO_MANY           = 7,  /* Can't handle this many actions. */
    OFPBAC_BAD_QUEUE          = 8,  /* Problem validating output queue. */
    OFPBAC_BAD_OUT_GROUP      = 9,  /* Invalid group id in forward action. */
    OFPBAC_MATCH_INCONSISTENT = 10, /* Action can't apply for this match,
                                       or Set-Field missing prerequisite. */
    OFPBAC_UNSUPPORTED_ORDER  = 11, /* Action order is unsupported for the
                                 action list in an Apply-Actions instruction */
    OFPBAC_BAD_TAG            = 12, /* Actions uses an unsupported
                                       tag/encap. */
    OFPBAC_BAD_SET_TYPE       = 13, /* Unsupported type in SET_FIELD action. */
    OFPBAC_BAD_SET_LEN        = 14, /* Length problem in SET_FIELD action. */
    OFPBAC_BAD_SET_ARGUMENT   = 15, /* Bad argument in SET_FIELD action. */
};

/* ofp_error_msg 'code' values for OFPET_BAD_INSTRUCTION.  'data' contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_instruction_code {
    OFPBIC_UNKNOWN_INST     = 0, /* Unknown instruction. */
    OFPBIC_UNSUP_INST       = 1, /* Switch or table does not support the
                                    instruction. */
    OFPBIC_BAD_TABLE_ID     = 2, /* Invalid Table-ID specified. */
    OFPBIC_UNSUP_METADATA   = 3, /* Metadata value unsupported by datapath. */
    OFPBIC_UNSUP_METADATA_MASK = 4, /* Metadata mask value unsupported by
                                       datapath. */
    OFPBIC_BAD_EXPERIMENTER = 5, /* Unknown experimenter id specified. */
    OFPBIC_BAD_EXP_TYPE     = 6, /* Unknown instruction for experimenter id. */
    OFPBIC_BAD_LEN          = 7, /* Length problem in instructions. */
    OFPBIC_EPERM            = 8, /* Permissions error. */
    OFPBIC_DUP_INST         = 9, /* Duplicate instruction. */
};

/* ofp_error_msg 'code' values for OFPET_BAD_MATCH.  'data' contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_match_code {
    OFPBMC_BAD_TYPE         = 0,  /* Unsupported match type specified by the
                                     match */
    OFPBMC_BAD_LEN          = 1,  /* Length problem in match. */
    OFPBMC_BAD_TAG          = 2,  /* Match uses an unsupported tag/encap. */
    OFPBMC_BAD_DL_ADDR_MASK = 3,  /* Unsupported datalink addr mask - switch
                                     does not support arbitrary datalink
                                     address mask. */
    OFPBMC_BAD_NW_ADDR_MASK = 4,  /* Unsupported network addr mask - switch
                                     does not support arbitrary network
                                     address mask. */
    OFPBMC_BAD_WILDCARDS    = 5,  /* Unsupported combination of fields masked
                                     or omitted in the match. */
    OFPBMC_BAD_FIELD        = 6,  /* Unsupported field type in the match. */
    OFPBMC_BAD_VALUE        = 7,  /* Unsupported value in a match field. */
    OFPBMC_BAD_MASK         = 8,  /* Unsupported mask specified in the match,
                                     field is not dl-address or nw-address. */
    OFPBMC_BAD_PREREQ       = 9,  /* A prerequisite was not met. */
    OFPBMC_DUP_FIELD        = 10, /* A field type was duplicated. */
    OFPBMC_EPERM            = 11, /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_FLOW_MOD_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_flow_mod_failed_code {
    OFPFMFC_UNKNOWN      = 0,   /* Unspecified error. */
    OFPFMFC_TABLE_FULL   = 1,   /* Flow not added because table was full. */
    OFPFMFC_BAD_TABLE_ID = 2,   /* Table does not exist */
    OFPFMFC_OVERLAP      = 3,   /* Attempted to add overlapping flow with
                                   CHECK_OVERLAP flag set. */
    OFPFMFC_EPERM        = 4,   /* Permissions error. */
    OFPFMFC_BAD_TIMEOUT  = 5,   /* Flow not added because of unsupported
                                   idle/hard timeout. */
    OFPFMFC_BAD_COMMAND  = 6,   /* Unsupported or unknown command. */
    OFPFMFC_BAD_FLAGS    = 7,   /* Unsupported or unknown flags. */
    OFPFMFC_CANT_SYNC    = 8,   /* Problem in table synchronisation. */
    OFPFMFC_BAD_PRIORITY = 9,   /* Unsupported priority value. */
    OFPFMFC_IS_SYNC      = 10,  /* Synchronised flow entry is read only. */
};

/* ofp_error_msg 'code' values for OFPET_GROUP_MOD_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_group_mod_failed_code {
    OFPGMFC_GROUP_EXISTS         = 0,  /* Group not added because a group ADD
                                          attempted to replace an
                                          already-present group. */
    OFPGMFC_INVALID_GROUP        = 1,  /* Group not added because Group
                                          specified is invalid. */
    OFPGMFC_WEIGHT_UNSUPPORTED   = 2,  /* Switch does not support unequal load
                                          sharing with select groups. */
    OFPGMFC_OUT_OF_GROUPS        = 3,  /* The group table is full. */
    OFPGMFC_OUT_OF_BUCKETS       = 4,  /* The maximum number of action buckets
                                          for a group has been exceeded. */
    OFPGMFC_CHAINING_UNSUPPORTED = 5,  /* Switch does not support groups that
                                          forward to groups. */
    OFPGMFC_WATCH_UNSUPPORTED    = 6,  /* This group cannot watch the watch_port
                                          or watch_group specified. */
    OFPGMFC_LOOP                 = 7,  /* Group entry would cause a loop. */
    OFPGMFC_UNKNOWN_GROUP        = 8,  /* Group not modified because a group
                                          MODIFY attempted to modify a
                                          non-existent group. */
    OFPGMFC_CHAINED_GROUP        = 9,  /* Group not deleted because another
                                          group is forwarding to it. */
    OFPGMFC_BAD_TYPE             = 10, /* Unsupported or unknown group type. */
    OFPGMFC_BAD_COMMAND          = 11, /* Unsupported or unknown command. */
    OFPGMFC_BAD_BUCKET           = 12, /* Error in bucket. */
    OFPGMFC_BAD_WATCH            = 13, /* Error in watch port/group. */
    OFPGMFC_EPERM                = 14, /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_PORT_MOD_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_port_mod_failed_code {
    OFPPMFC_BAD_PORT      = 0,   /* Specified port number does not exist. */
    OFPPMFC_BAD_HW_ADDR   = 1,   /* Specified hardware address does not
                                  * match the port number. */
    OFPPMFC_BAD_CONFIG    = 2,   /* Specified config is invalid. */
    OFPPMFC_BAD_ADVERTISE = 3,   /* Specified advertise is invalid. */
    OFPPMFC_EPERM         = 4,   /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_TABLE_MOD_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_table_mod_failed_code {
    OFPTMFC_BAD_TABLE  = 0,      /* Specified table does not exist. */
    OFPTMFC_BAD_CONFIG = 1,      /* Specified config is invalid. */
    OFPTMFC_EPERM      = 2,      /* Permissions error. */
};

/* ofp_error msg 'code' values for OFPET_QUEUE_OP_FAILED. 'data' contains
 * at least the first 64 bytes of the failed request */
enum ofp_queue_op_failed_code {
    OFPQOFC_BAD_PORT   = 0,     /* Invalid port (or port does not exist). */
    OFPQOFC_BAD_QUEUE  = 1,     /* Queue does not exist. */
    OFPQOFC_EPERM      = 2,     /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_SWITCH_CONFIG_FAILED. 'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_switch_config_failed_code {
    OFPSCFC_BAD_FLAGS  = 0,      /* Specified flags is invalid. */
    OFPSCFC_BAD_LEN    = 1,      /* Specified len is invalid. */
    OFPSCFC_EPERM      = 2,      /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_ROLE_REQUEST_FAILED. 'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_role_request_failed_code {
    OFPRRFC_STALE      = 0,      /* Stale Message: old generation_id. */
    OFPRRFC_UNSUP      = 1,      /* Controller role change unsupported. */
    OFPRRFC_BAD_ROLE   = 2,      /* Invalid role. */
};

/* ofp_error_msg 'code' values for OFPET_METER_MOD_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_meter_mod_failed_code {
    OFPMMFC_UNKNOWN       = 0,  /* Unspecified error. */
    OFPMMFC_METER_EXISTS  = 1,  /* Meter not added because a Meter ADD
                                 * attempted to replace an existing Meter. */
    OFPMMFC_INVALID_METER = 2,  /* Meter not added because Meter specified
                                 * is invalid,
                                 * or invalid meter in meter action. */
    OFPMMFC_UNKNOWN_METER = 3,  /* Meter not modified because a Meter MODIFY
                                 * attempted to modify a non-existent Meter,
                                 * or bad meter in meter action. */
    OFPMMFC_BAD_COMMAND   = 4,  /* Unsupported or unknown command. */
    OFPMMFC_BAD_FLAGS     = 5,  /* Flag configuration unsupported. */
    OFPMMFC_BAD_RATE      = 6,  /* Rate unsupported. */
    OFPMMFC_BAD_BURST     = 7,  /* Burst size unsupported. */
    OFPMMFC_BAD_BAND      = 8,  /* Band unsupported. */
    OFPMMFC_BAD_BAND_VALUE = 9, /* Band value unsupported. */
    OFPMMFC_OUT_OF_METERS = 10, /* No more meters available. */
    OFPMMFC_OUT_OF_BANDS  = 11, /* The maximum number of properties
                                 * for a meter has been exceeded. */
};

/* ofp_error_msg 'code' values for OFPET_TABLE_FEATURES_FAILED. 'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_table_features_failed_code {
    OFPTFFC_BAD_TABLE    = 0,      /* Specified table does not exist. */
    OFPTFFC_BAD_METADATA = 1,      /* Invalid metadata mask. */
    OFPTFFC_EPERM        = 5,      /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_BAD_PROPERTY. 'data' contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_property_code {
    OFPBPC_BAD_TYPE           = 0,  /* Unknown property type. */
    OFPBPC_BAD_LEN            = 1,  /* Length problem in property. */
    OFPBPC_BAD_VALUE          = 2,  /* Unsupported property value. */
    OFPBPC_TOO_MANY           = 3,  /* Can't handle this many properties. */
    OFPBPC_DUP_TYPE           = 4,  /* A property type was duplicated. */
    OFPBPC_BAD_EXPERIMENTER   = 5,  /* Unknown experimenter id specified. */
    OFPBPC_BAD_EXP_TYPE       = 6,  /* Unknown exp_type for experimenter id. */
    OFPBPC_BAD_EXP_VALUE      = 7,  /* Unknown value for experimenter id. */
    OFPBPC_EPERM              = 8,  /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_ASYNC_CONFIG_FAILED. 'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_async_config_failed_code {
    OFPACFC_INVALID      = 0,      /* One mask is invalid. */
    OFPACFC_UNSUPPORTED  = 1,      /* Requested configuration not supported. */
    OFPACFC_EPERM        = 2,      /* Permissions error. */
};

/* ofp_error_msg 'code' values for OFPET_FLOW_MONITOR_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_flow_monitor_failed_code {
    OFPMOFC_UNKNOWN       = 0,  /* Unspecified error. */
    OFPMOFC_MONITOR_EXISTS = 1, /* Monitor not added because a Monitor ADD
                                 * attempted to replace an existing Monitor. */
    OFPMOFC_INVALID_MONITOR = 2, /* Monitor not added because Monitor specified
                                 * is invalid. */
    OFPMOFC_UNKNOWN_MONITOR = 3, /* Monitor not modified because a Monitor
                                   MODIFY attempted to modify a non-existent
                                   Monitor. */
    OFPMOFC_BAD_COMMAND   = 4,  /* Unsupported or unknown command. */
    OFPMOFC_BAD_FLAGS     = 5,  /* Flag configuration unsupported. */
    OFPMOFC_BAD_TABLE_ID  = 6,  /* Specified table does not exist. */
    OFPMOFC_BAD_OUT       = 7,  /* Error in output port/group. */
};

/* ofp_error_msg 'code' values for OFPET_BUNDLE_FAILED.  'data' contains
 * at least the first 64 bytes of the failed request. */
enum ofp_bundle_failed_code {
  OFPBFC_UNKNOWN        = 0,  /* Unspecified error. */
  OFPBFC_EPERM          = 1,  /* Permissions error. */
  OFPBFC_BAD_ID         = 2,  /* Bundle ID doesn't exist. */
  OFPBFC_BUNDLE_EXIST   = 3,  /* Bundle ID already exist. */
  OFPBFC_BUNDLE_CLOSED  = 4,  /* Bundle ID is closed. */
  OFPBFC_OUT_OF_BUNDLES = 5,  /* Too many bundles IDs. */
  OFPBFC_BAD_TYPE       = 6,  /* Unsupported or unknown message control type. */
  OFPBFC_BAD_FLAGS      = 7,  /* Unsupported, unknown, or inconsistent flags. */
  OFPBFC_MSG_BAD_LEN    = 8,  /* Length problem in included message. */
  OFPBFC_MSG_BAD_XID    = 9,  /* Inconsistent or duplicate XID. */
  OFPBFC_MSG_UNSUP      = 10, /* Unsupported message in this bundle. */
  OFPBFC_MSG_CONFLICT   = 11, /* Unsupported message combination in this bundle. */
  OFPBFC_MSG_TOO_MANY   = 12, /* Can't handle this many messages in bundle. */
  OFPBFC_MSG_FAILED     = 13, /* One message in bundle failed. */
  OFPBFC_TIMEOUT        = 14, /* Bundle is taking too long. */
  OFPBFC_BUNDLE_IN_PROGRESS = 15, /* Bundle is locking the resource. */
};

/* OFPT_ERROR: Error message. */
struct ofp_error_msg {
    struct ofp_header header;

    big_uint16_t type;
    big_uint16_t code;
    big_uint8_t data[0];          /* Variable-length data.  Interpreted based
                                 on the type and code.  No padding. */
};
OFP_ASSERT(sizeof(struct ofp_error_msg) == 12);

/* OFPET_EXPERIMENTER: Error message (datapath -> controller). */
struct ofp_error_experimenter_msg {
    struct ofp_header header;

    big_uint16_t type;            /* OFPET_EXPERIMENTER. */
    big_uint16_t exp_code;        /* Experimenter defined. */
    big_uint32_t experimenter;    /* Experimenter ID. */
    big_uint8_t data[0];          /* Variable-length data.  Interpreted based
                                 on the type and experimenter.  No padding. */
};
OFP_ASSERT(sizeof(struct ofp_error_experimenter_msg) == 16);

enum ofp_multipart_type {
    /* Description of this OpenFlow switch.
     * The request body is empty.
     * The reply body is struct ofp_desc. */
    OFPMP_DESC = 0,

    /* Individual flow statistics.
     * The request body is struct ofp_flow_stats_request.
     * The reply body is an array of struct ofp_flow_stats. */
    OFPMP_FLOW = 1,

    /* Aggregate flow statistics.
     * The request body is struct ofp_aggregate_stats_request.
     * The reply body is struct ofp_aggregate_stats_reply. */
    OFPMP_AGGREGATE = 2,

    /* Flow table statistics.
     * The request body is empty.
     * The reply body is an array of struct ofp_table_stats. */
    OFPMP_TABLE = 3,

    /* Port statistics.
     * The request body is struct ofp_port_stats_request.
     * The reply body is an array of struct ofp_port_stats. */
    OFPMP_PORT_STATS = 4,

    /* Queue statistics for a port
     * The request body is struct ofp_queue_stats_request.
     * The reply body is an array of struct ofp_queue_stats */
    OFPMP_QUEUE_STATS = 5,

    /* Group counter statistics.
     * The request body is struct ofp_group_stats_request.
     * The reply is an array of struct ofp_group_stats. */
    OFPMP_GROUP = 6,

    /* Group description.
     * The request body is empty.
     * The reply body is an array of struct ofp_group_desc. */
    OFPMP_GROUP_DESC = 7,

    /* Group features.
     * The request body is empty.
     * The reply body is struct ofp_group_features. */
    OFPMP_GROUP_FEATURES = 8,

    /* Meter statistics.
     * The request body is struct ofp_meter_multipart_requests.
     * The reply body is an array of struct ofp_meter_stats. */
    OFPMP_METER = 9,

    /* Meter configuration.
     * The request body is struct ofp_meter_multipart_requests.
     * The reply body is an array of struct ofp_meter_config. */
    OFPMP_METER_CONFIG = 10,

    /* Meter features.
     * The request body is empty.
     * The reply body is struct ofp_meter_features. */
    OFPMP_METER_FEATURES = 11,

    /* Table features.
     * The request body is either empty or contains an array of
     * struct ofp_table_features containing the controller's
     * desired view of the switch. If the switch is unable to
     * set the specified view an error is returned.
     * The reply body is an array of struct ofp_table_features. */
    OFPMP_TABLE_FEATURES = 12,

    /* Port description.
     * The request body is empty.
     * The reply body is an array of struct ofp_port. */
    OFPMP_PORT_DESC = 13,

    /* Table description.
     * The request body is empty.
     * The reply body is an array of struct ofp_table_desc. */
    OFPMP_TABLE_DESC = 14,

    /* Queue description.
     * The request body is struct ofp_queue_desc_request.
     * The reply body is an array of struct ofp_queue_desc. */
    OFPMP_QUEUE_DESC = 15,

    /* Flow monitors. Reply may be an asynchronous message.
     * The request body is an array of struct ofp_flow_monitor_request.
     * The reply body is an array of struct ofp_flow_update_header. */
    OFPMP_FLOW_MONITOR = 16,

    /* Experimenter extension.
     * The request and reply bodies begin with
     * struct ofp_experimenter_multipart_header.
     * The request and reply bodies are otherwise experimenter-defined. */
    OFPMP_EXPERIMENTER = 0xffff
};

/* Backward compatibility with 1.3.1 - avoid breaking the API. */
#define ofp_multipart_types ofp_multipart_type

enum ofp_multipart_request_flags {
    OFPMPF_REQ_MORE  = 1 << 0  /* More requests to follow. */
};

struct ofp_multipart_request {
    struct ofp_header header;
    big_uint16_t type;              /* One of the OFPMP_* constants. */
    big_uint16_t flags;             /* OFPMPF_REQ_* flags. */
    big_uint8_t pad[4];
    big_uint8_t body[0];            /* Body of the request. 0 or more bytes. */
};
OFP_ASSERT(sizeof(struct ofp_multipart_request) == 16);

enum ofp_multipart_reply_flags {
    OFPMPF_REPLY_MORE  = 1 << 0  /* More replies to follow. */
};

struct ofp_multipart_reply {
    struct ofp_header header;
    big_uint16_t type;              /* One of the OFPMP_* constants. */
    big_uint16_t flags;             /* OFPMPF_REPLY_* flags. */
    big_uint8_t pad[4];
    big_uint8_t body[0];            /* Body of the reply. 0 or more bytes. */
};
OFP_ASSERT(sizeof(struct ofp_multipart_reply) == 16);

#define DESC_STR_LEN   256
#define SERIAL_NUM_LEN 32
/* Body of reply to OFPMP_DESC request.  Each entry is a NULL-terminated
 * ASCII string. */
struct ofp_desc {
    char mfr_desc[DESC_STR_LEN];       /* Manufacturer description. */
    char hw_desc[DESC_STR_LEN];        /* Hardware description. */
    char sw_desc[DESC_STR_LEN];        /* Software description. */
    char serial_num[SERIAL_NUM_LEN];   /* Serial number. */
    char dp_desc[DESC_STR_LEN];        /* Human readable description of datapath. */
};
OFP_ASSERT(sizeof(struct ofp_desc) == 1056);

/* Body for ofp_multipart_request of type OFPMP_FLOW. */
struct ofp_flow_stats_request {
    big_uint8_t table_id;         /* ID of table to read (from ofp_table_stats),
                                 OFPTT_ALL for all tables. */
    big_uint8_t pad[3];           /* Align to 32 bits. */
    big_uint32_t out_port;        /* Require matching entries to include this
                                 as an output port.  A value of OFPP_ANY
                                 indicates no restriction. */
    big_uint32_t out_group;       /* Require matching entries to include this
                                 as an output group.  A value of OFPG_ANY
                                 indicates no restriction. */
    big_uint8_t pad2[4];          /* Align to 64 bits. */
    big_uint64_t cookie;          /* Require matching entries to contain this
                                 cookie value */
    big_uint64_t cookie_mask;     /* Mask used to restrict the cookie bits that
                                 must match. A value of 0 indicates
                                 no restriction. */
    struct ofp_match match;   /* Fields to match. Variable size. */
};
OFP_ASSERT(sizeof(struct ofp_flow_stats_request) == 40);

/* Body of reply to OFPMP_FLOW request. */
struct ofp_flow_stats {
    big_uint16_t length;          /* Length of this entry. */
    big_uint8_t table_id;         /* ID of table flow came from. */
    big_uint8_t pad;
    big_uint32_t duration_sec;    /* Time flow has been alive in seconds. */
    big_uint32_t duration_nsec;   /* Time flow has been alive in nanoseconds beyond
                                 duration_sec. */
    big_uint16_t priority;        /* Priority of the entry. */
    big_uint16_t idle_timeout;    /* Number of seconds idle before expiration. */
    big_uint16_t hard_timeout;    /* Number of seconds before expiration. */
    big_uint16_t flags;           /* Bitmap of OFPFF_* flags. */
    big_uint16_t importance;      /* Eviction precedence. */
    big_uint8_t pad2[2];          /* Align to 64-bits. */
    big_uint64_t cookie;          /* Opaque controller-issued identifier. */
    big_uint64_t packet_count;    /* Number of packets in flow. */
    big_uint64_t byte_count;      /* Number of bytes in flow. */
    struct ofp_match match;   /* Description of fields. Variable size. */
    //struct ofp_instruction_header instructions[0];
                              /* Instruction set - 0 or more. */
};
OFP_ASSERT(sizeof(struct ofp_flow_stats) == 56);

/* Body for ofp_multipart_request of type OFPMP_AGGREGATE. */
struct ofp_aggregate_stats_request {
    big_uint8_t table_id;         /* ID of table to read (from ofp_table_stats)
                                 OFPTT_ALL for all tables. */
    big_uint8_t pad[3];           /* Align to 32 bits. */
    big_uint32_t out_port;        /* Require matching entries to include this
                                 as an output port.  A value of OFPP_ANY
                                 indicates no restriction. */
    big_uint32_t out_group;       /* Require matching entries to include this
                                 as an output group.  A value of OFPG_ANY
                                 indicates no restriction. */
    big_uint8_t pad2[4];          /* Align to 64 bits. */
    big_uint64_t cookie;          /* Require matching entries to contain this
                                 cookie value */
    big_uint64_t cookie_mask;     /* Mask used to restrict the cookie bits that
                                 must match. A value of 0 indicates
                                 no restriction. */
    struct ofp_match match;   /* Fields to match. Variable size. */
};
OFP_ASSERT(sizeof(struct ofp_aggregate_stats_request) == 40);

/* Body of reply to OFPMP_AGGREGATE request. */
struct ofp_aggregate_stats_reply {
    big_uint64_t packet_count;    /* Number of packets in flows. */
    big_uint64_t byte_count;      /* Number of bytes in flows. */
    big_uint32_t flow_count;      /* Number of flows. */
    big_uint8_t pad[4];           /* Align to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_aggregate_stats_reply) == 24);

/* Table Feature property types.
 * Low order bit cleared indicates a property for a regular Flow Entry.
 * Low order bit set indicates a property for the Table-Miss Flow Entry.
 */
enum ofp_table_feature_prop_type {
    OFPTFPT_INSTRUCTIONS           = 0,  /* Instructions property. */
    OFPTFPT_INSTRUCTIONS_MISS      = 1,  /* Instructions for table-miss. */
    OFPTFPT_NEXT_TABLES            = 2,  /* Next Table property. */
    OFPTFPT_NEXT_TABLES_MISS       = 3,  /* Next Table for table-miss. */
    OFPTFPT_WRITE_ACTIONS          = 4,  /* Write Actions property. */
    OFPTFPT_WRITE_ACTIONS_MISS     = 5,  /* Write Actions for table-miss. */
    OFPTFPT_APPLY_ACTIONS          = 6,  /* Apply Actions property. */
    OFPTFPT_APPLY_ACTIONS_MISS     = 7,  /* Apply Actions for table-miss. */
    OFPTFPT_MATCH                  = 8,  /* Match property. */
    OFPTFPT_WILDCARDS              = 10, /* Wildcards property. */
    OFPTFPT_WRITE_SETFIELD         = 12, /* Write Set-Field property. */
    OFPTFPT_WRITE_SETFIELD_MISS    = 13, /* Write Set-Field for table-miss. */
    OFPTFPT_APPLY_SETFIELD         = 14, /* Apply Set-Field property. */
    OFPTFPT_APPLY_SETFIELD_MISS    = 15, /* Apply Set-Field for table-miss. */
    OFPTFPT_TABLE_SYNC_FROM        = 16, /* Table synchronisation property. */
    OFPTFPT_EXPERIMENTER           = 0xFFFE, /* Experimenter property. */
    OFPTFPT_EXPERIMENTER_MISS      = 0xFFFF, /* Experimenter for table-miss. */
};

/* Common header for all Table Feature Properties */
struct ofp_table_feature_prop_header {
    big_uint16_t         type;    /* One of OFPTFPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_header) == 4);

/* Instruction ID */
struct ofp_instruction_id {
    big_uint16_t type;                /* One of OFPIT_*. */
    big_uint16_t len;                 /* Length is 4 or experimenter defined. */
    big_uint8_t exp_data[0];          /* Optional experimenter id + data. */
};
OFP_ASSERT(sizeof(struct ofp_instruction_id) == 4);

/* Instructions property */
struct ofp_table_feature_prop_instructions {
    big_uint16_t         type;    /* One of OFPTFPT_INSTRUCTIONS,
                                 OFPTFPT_INSTRUCTIONS_MISS. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    /* Followed by:
     *   - Exactly (length - 4) bytes containing the instruction ids, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    struct ofp_instruction_id  instruction_ids[0];   /* List of instructions */
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_instructions) == 4);

/* Next Tables and Table Synchronise From properties */
struct ofp_table_feature_prop_tables {
    big_uint16_t         type;    /* One of OFPTFPT_NEXT_TABLES,
                                 OFPTFPT_NEXT_TABLES_MISS,
                                 OFPTFPT_TABLE_SYNC_FROM. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    /* Followed by:
     *   - Exactly (length - 4) bytes containing the table_ids, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint8_t          table_ids[0];        /* List of table ids. */
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_tables) == 4);

/* Action ID */
struct ofp_action_id {
    big_uint16_t type;                /* One of OFPAT_*. */
    big_uint16_t len;                 /* Length is 4 or experimenter defined. */
    big_uint8_t exp_data[0];          /* Optional experimenter id + data. */
};
OFP_ASSERT(sizeof(struct ofp_action_id) == 4);

/* Actions property */
struct ofp_table_feature_prop_actions {
    big_uint16_t         type;    /* One of OFPTFPT_WRITE_ACTIONS,
                                 OFPTFPT_WRITE_ACTIONS_MISS,
                                 OFPTFPT_APPLY_ACTIONS,
                                 OFPTFPT_APPLY_ACTIONS_MISS. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    /* Followed by:
     *   - Exactly (length - 4) bytes containing the action_ids, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    struct ofp_action_id  action_ids[0];      /* List of actions */
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_actions) == 4);

/* Match, Wildcard or Set-Field property */
struct ofp_table_feature_prop_oxm {
    big_uint16_t         type;    /* One of OFPTFPT_MATCH,
                                 OFPTFPT_WILDCARDS,
                                 OFPTFPT_WRITE_SETFIELD,
                                 OFPTFPT_WRITE_SETFIELD_MISS,
                                 OFPTFPT_APPLY_SETFIELD,
                                 OFPTFPT_APPLY_SETFIELD_MISS. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    /* Followed by:
     *   - Exactly (length - 4) bytes containing the oxm_ids, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         oxm_ids[0];   /* Array of OXM headers */
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_oxm) == 4);

/* Experimenter table feature property */
struct ofp_table_feature_prop_experimenter {
    big_uint16_t         type;    /* One of OFPTFPT_EXPERIMENTER,
                                 OFPTFPT_EXPERIMENTER_MISS. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_table_feature_prop_experimenter) == 12);

/* Body for ofp_multipart_request of type OFPMP_TABLE_FEATURES./
 * Body of reply to OFPMP_TABLE_FEATURES request. */
struct ofp_table_features {
    big_uint16_t length;         /* Length is padded to 64 bits. */
    big_uint8_t table_id;        /* Identifier of table.  Lower numbered tables
                                are consulted first. */
    big_uint8_t pad[5];          /* Align to 64-bits. */
    char name[OFP_MAX_TABLE_NAME_LEN];
    big_uint64_t metadata_match; /* Bits of metadata table can match. */
    big_uint64_t metadata_write; /* Bits of metadata table can write. */
    big_uint32_t capabilities;   /* Bitmap of OFPTC_* values. */
    big_uint32_t max_entries;    /* Max number of entries supported. */

    /* Table Feature Property list */
    struct ofp_table_feature_prop_header properties[0]; /* List of properties */
};
OFP_ASSERT(sizeof(struct ofp_table_features) == 64);

/* Body of reply to OFPMP_TABLE request. */
struct ofp_table_stats {
    big_uint8_t table_id;        /* Identifier of table.  Lower numbered tables
                                are consulted first. */
    big_uint8_t pad[3];          /* Align to 32-bits. */
    big_uint32_t active_count;   /* Number of active entries. */
    big_uint64_t lookup_count;   /* Number of packets looked up in table. */
    big_uint64_t matched_count;  /* Number of packets that hit table. */
};
OFP_ASSERT(sizeof(struct ofp_table_stats) == 24);

/* Body of reply to OFPMP_TABLE_DESC request. */
struct ofp_table_desc {
    big_uint16_t length;         /* Length is padded to 64 bits. */
    big_uint8_t table_id;        /* Identifier of table.  Lower numbered tables
                                are consulted first. */
    big_uint8_t pad[1];          /* Align to 32-bits. */
    big_uint32_t config;         /* Bitmap of OFPTC_* values. */

    /* Table Mod Property list - 0 or more. */
    struct ofp_table_mod_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_table_desc) == 8);

/* Body for ofp_multipart_request of type OFPMP_PORT_STATS. */
struct ofp_port_stats_request {
    big_uint32_t port_no;        /* OFPMP_PORT message must request statistics
                              * either for a single port (specified in
                              * port_no) or for all ports (if port_no ==
                              * OFPP_ANY). */
    big_uint8_t pad[4];
};
OFP_ASSERT(sizeof(struct ofp_port_stats_request) == 8);

/* Port stats property types.
 */
enum ofp_port_stats_prop_type {
    OFPPSPT_ETHERNET          = 0,      /* Ethernet property. */
    OFPPSPT_OPTICAL           = 1,      /* Optical property. */
    OFPPSPT_EXPERIMENTER      = 0xFFFF, /* Experimenter property. */
};

/* Common header for all port stats properties. */
struct ofp_port_stats_prop_header {
    big_uint16_t         type;    /* One of OFPPSPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_port_stats_prop_header) == 4);

/* Ethernet port stats property. */
struct ofp_port_stats_prop_ethernet {
    big_uint16_t         type;    /* OFPPSPT_ETHERNET. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint8_t          pad[4];  /* Align to 64 bits. */

    big_uint64_t rx_frame_err;   /* Number of frame alignment errors. */
    big_uint64_t rx_over_err;    /* Number of packets with RX overrun. */
    big_uint64_t rx_crc_err;     /* Number of CRC errors. */
    big_uint64_t collisions;     /* Number of collisions. */
};
OFP_ASSERT(sizeof(struct ofp_port_stats_prop_ethernet) == 40);

/* Optical port stats property. */
struct ofp_port_stats_prop_optical {
    big_uint16_t         type;    /* OFPPSPT_OPTICAL. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint8_t          pad[4];  /* Align to 64 bits. */

    big_uint32_t flags;          /* Features enabled by the port. */
    big_uint32_t tx_freq_lmda;   /* Current TX Frequency/Wavelength */
    big_uint32_t tx_offset;      /* TX Offset */
    big_uint32_t tx_grid_span;   /* TX Grid Spacing */
    big_uint32_t rx_freq_lmda;   /* Current RX Frequency/Wavelength */
    big_uint32_t rx_offset;      /* RX Offset */
    big_uint32_t rx_grid_span;   /* RX Grid Spacing */
    big_uint16_t tx_pwr;         /* Current TX power */
    big_uint16_t rx_pwr;         /* Current RX power */
    big_uint16_t bias_current;   /* TX Bias Current */
    big_uint16_t temperature;    /* TX Laser Temperature */
};
OFP_ASSERT(sizeof(struct ofp_port_stats_prop_optical) == 44);

/* Flags is one of OFPOSF_ below */
enum ofp_port_stats_optical_flags  {
    OFPOSF_RX_TUNE   = 1 << 0,  /* Receiver tune info valid */
    OFPOSF_TX_TUNE   = 1 << 1,  /* Transmit tune info valid */
    OFPOSF_TX_PWR    = 1 << 2,  /* TX Power is valid */
    OFPOSF_RX_PWR    = 1 << 4,  /* RX power is valid */
    OFPOSF_TX_BIAS   = 1 << 5,  /* Transmit bias is valid */
    OFPOSF_TX_TEMP   = 1 << 6,  /* TX Temp is valid */
};

/* Experimenter port stats property. */
struct ofp_port_stats_prop_experimenter {
    big_uint16_t         type;    /* OFPPSPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_port_stats_prop_experimenter) == 12);

/* Body of reply to OFPMP_PORT_STATS request. If a counter is unsupported,
 * set the field to all ones. */
struct ofp_port_stats {
    big_uint16_t length;         /* Length of this entry. */
    big_uint8_t pad[2];          /* Align to 64 bits. */
    big_uint32_t port_no;
    big_uint32_t duration_sec;   /* Time port has been alive in seconds. */
    big_uint32_t duration_nsec;  /* Time port has been alive in nanoseconds beyond
                                duration_sec. */
    big_uint64_t rx_packets;     /* Number of received packets. */
    big_uint64_t tx_packets;     /* Number of transmitted packets. */
    big_uint64_t rx_bytes;       /* Number of received bytes. */
    big_uint64_t tx_bytes;       /* Number of transmitted bytes. */

    big_uint64_t rx_dropped;     /* Number of packets dropped by RX. */
    big_uint64_t tx_dropped;     /* Number of packets dropped by TX. */
    big_uint64_t rx_errors;      /* Number of receive errors.  This is a super-set
                                of more specific receive errors and should be
                                greater than or equal to the sum of all
                                rx_*_err values in properties. */
    big_uint64_t tx_errors;      /* Number of transmit errors.  This is a super-set
                                of more specific transmit errors and should be
                                greater than or equal to the sum of all
                                tx_*_err values (none currently defined.) */

    /* Port description property list - 0 or more properties */
    struct ofp_port_desc_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_port_stats) == 80);

/* Body of OFPMP_GROUP request. */
struct ofp_group_stats_request {
    big_uint32_t group_id;       /* All groups if OFPG_ALL. */
    big_uint8_t pad[4];          /* Align to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_group_stats_request) == 8);

/* Used in group stats replies. */
struct ofp_bucket_counter {
    big_uint64_t packet_count;   /* Number of packets processed by bucket. */
    big_uint64_t byte_count;     /* Number of bytes processed by bucket. */
};
OFP_ASSERT(sizeof(struct ofp_bucket_counter) == 16);

/* Body of reply to OFPMP_GROUP request. */
struct ofp_group_stats {
    big_uint16_t length;         /* Length of this entry. */
    big_uint8_t pad[2];          /* Align to 64 bits. */
    big_uint32_t group_id;       /* Group identifier. */
    big_uint32_t ref_count;      /* Number of flows or groups that directly forward
                                to this group. */
    big_uint8_t pad2[4];         /* Align to 64 bits. */
    big_uint64_t packet_count;   /* Number of packets processed by group. */
    big_uint64_t byte_count;     /* Number of bytes processed by group. */
    big_uint32_t duration_sec;   /* Time group has been alive in seconds. */
    big_uint32_t duration_nsec;  /* Time group has been alive in nanoseconds beyond
                                duration_sec. */
    struct ofp_bucket_counter bucket_stats[0]; /* One counter set per bucket. */
};
OFP_ASSERT(sizeof(struct ofp_group_stats) == 40);

/* Body of reply to OFPMP_GROUP_DESC request. */
struct ofp_group_desc {
    big_uint16_t length;              /* Length of this entry. */
    big_uint8_t type;                 /* One of OFPGT_*. */
    big_uint8_t pad;                  /* Pad to 64 bits. */
    big_uint32_t group_id;            /* Group identifier. */
    struct ofp_bucket buckets[0];   /* List of buckets - 0 or more. */
};
OFP_ASSERT(sizeof(struct ofp_group_desc) == 8);

/* Backward compatibility with 1.3.1 - avoid breaking the API. */
#define ofp_group_desc_stats ofp_group_desc

/* Group configuration flags */
enum ofp_group_capabilities {
    OFPGFC_SELECT_WEIGHT   = 1 << 0,  /* Support weight for select groups */
    OFPGFC_SELECT_LIVENESS = 1 << 1,  /* Support liveness for select groups */
    OFPGFC_CHAINING        = 1 << 2,  /* Support chaining groups */
    OFPGFC_CHAINING_CHECKS = 1 << 3,  /* Check chaining for loops and delete */
};

/* Body of reply to OFPMP_GROUP_FEATURES request. Group features. */
struct ofp_group_features {
    big_uint32_t  types;           /* Bitmap of (1 << OFPGT_*) values supported. */
    big_uint32_t  capabilities;    /* Bitmap of OFPGFC_* capability supported. */
    big_uint32_t  max_groups[4];   /* Maximum number of groups for each type. */
    big_uint32_t  actions[4];      /* Bitmaps of (1 << OFPAT_*) values supported. */
};
OFP_ASSERT(sizeof(struct ofp_group_features) == 40);

/* Body of OFPMP_METER and OFPMP_METER_CONFIG requests. */
struct ofp_meter_multipart_request {
    big_uint32_t meter_id;       /* Meter instance, or OFPM_ALL. */
    big_uint8_t pad[4];          /* Align to 64 bits. */
};
OFP_ASSERT(sizeof(struct ofp_meter_multipart_request) == 8);

/* Statistics for each meter band */
struct ofp_meter_band_stats {
    big_uint64_t        packet_band_count;   /* Number of packets in band. */
    big_uint64_t        byte_band_count;     /* Number of bytes in band. */
};
OFP_ASSERT(sizeof(struct ofp_meter_band_stats) == 16);

/* Body of reply to OFPMP_METER request. Meter statistics. */
struct ofp_meter_stats {
    big_uint32_t        meter_id;         /* Meter instance. */
    big_uint16_t        len;              /* Length in bytes of this stats. */
    big_uint8_t         pad[6];
    big_uint32_t        flow_count;       /* Number of flows bound to meter. */
    big_uint64_t        packet_in_count;  /* Number of packets in input. */
    big_uint64_t        byte_in_count;    /* Number of bytes in input. */
    big_uint32_t   duration_sec;  /* Time meter has been alive in seconds. */
    big_uint32_t   duration_nsec; /* Time meter has been alive in nanoseconds beyond
                                 duration_sec. */
    struct ofp_meter_band_stats band_stats[0]; /* The band_stats length is
                                         inferred from the length field. */
};
OFP_ASSERT(sizeof(struct ofp_meter_stats) == 40);

/* Body of reply to OFPMP_METER_CONFIG request. Meter configuration. */
struct ofp_meter_config {
    big_uint16_t        length;           /* Length of this entry. */
    big_uint16_t        flags;            /* All OFPMF_* that apply. */
    big_uint32_t        meter_id;         /* Meter instance. */
    struct ofp_meter_band_header bands[0]; /* The bands length is
                                         inferred from the length field. */
};
OFP_ASSERT(sizeof(struct ofp_meter_config) == 8);

/* Body of reply to OFPMP_METER_FEATURES request. Meter features. */
struct ofp_meter_features {
    big_uint32_t    max_meter;    /* Maximum number of meters. */
    big_uint32_t    band_types;   /* Bitmaps of (1 << OFPMBT_*) values supported. */
    big_uint32_t    capabilities; /* Bitmaps of "ofp_meter_flags". */
    big_uint8_t     max_bands;    /* Maximum bands per meters */
    big_uint8_t     max_color;    /* Maximum color value */
    big_uint8_t     pad[2];
};
OFP_ASSERT(sizeof(struct ofp_meter_features) == 16);

/* All ones is used to indicate all queues in a port (for stats retrieval). */
#define OFPQ_ALL      0xffffffff

/* Min rate > 1000 means not configured. */
#define OFPQ_MIN_RATE_UNCFG      0xffff

/* Max rate > 1000 means not configured. */
#define OFPQ_MAX_RATE_UNCFG      0xffff

enum ofp_queue_desc_prop_type {
    OFPQDPT_MIN_RATE      = 1,      /* Minimum datarate guaranteed. */
    OFPQDPT_MAX_RATE      = 2,      /* Maximum datarate. */
    OFPQDPT_EXPERIMENTER  = 0xffff  /* Experimenter defined property. */
};

/* Common header for all queue properties */
struct ofp_queue_desc_prop_header {
    big_uint16_t         type;    /* One of OFPQDPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_queue_desc_prop_header) == 4);

/* Min-Rate queue property description. */
struct ofp_queue_desc_prop_min_rate {
    big_uint16_t         type;    /* OFPQDPT_MIN_RATE. */
    big_uint16_t         length;  /* Length is 8. */
    big_uint16_t rate;        /* In 1/10 of a percent; >1000 -> disabled. */
    big_uint8_t pad[2];       /* 64-bit alignment */
};
OFP_ASSERT(sizeof(struct ofp_queue_desc_prop_min_rate) == 8);

/* Max-Rate queue property description. */
struct ofp_queue_desc_prop_max_rate {
    big_uint16_t         type;    /* OFPQDPT_MAX_RATE. */
    big_uint16_t         length;  /* Length is 8. */
    big_uint16_t rate;        /* In 1/10 of a percent; >1000 -> disabled. */
    big_uint8_t pad[2];       /* 64-bit alignment */
};
OFP_ASSERT(sizeof(struct ofp_queue_desc_prop_max_rate) == 8);

/* Experimenter queue property description. */
struct ofp_queue_desc_prop_experimenter {
    big_uint16_t         type;    /* OFPQDPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_queue_desc_prop_experimenter) == 12);

/* Body for ofp_multipart_request of type OFPMP_QUEUE_DESC. */
struct ofp_queue_desc_request {
    big_uint32_t port_no;        /* All ports if OFPP_ANY. */
    big_uint32_t queue_id;       /* All queues if OFPQ_ALL. */
};
OFP_ASSERT(sizeof(struct ofp_queue_desc_request) == 8);

/* Body of reply to OFPMP_QUEUE_DESC request. */
struct ofp_queue_desc {
    big_uint32_t port_no;      /* Port this queue is attached to. */
    big_uint32_t queue_id;     /* id for the specific queue. */
    big_uint16_t len;          /* Length in bytes of this queue desc. */
    big_uint8_t pad[6];        /* 64-bit alignment. */

    struct ofp_queue_desc_prop_header properties[0]; /* List of properties. */
};
OFP_ASSERT(sizeof(struct ofp_queue_desc) == 16);

/* Body for ofp_multipart_request of type OFPMP_QUEUE_STATS. */
struct ofp_queue_stats_request {
    big_uint32_t port_no;        /* All ports if OFPP_ANY. */
    big_uint32_t queue_id;       /* All queues if OFPQ_ALL. */
};
OFP_ASSERT(sizeof(struct ofp_queue_stats_request) == 8);

enum ofp_queue_stats_prop_type {
    OFPQSPT_EXPERIMENTER  = 0xffff  /* Experimenter defined property. */
};

/* Common header for all queue properties */
struct ofp_queue_stats_prop_header {
    big_uint16_t         type;    /* One of OFPQSPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_queue_stats_prop_header) == 4);

/* Experimenter queue property description. */
struct ofp_queue_stats_prop_experimenter {
    big_uint16_t         type;    /* OFPQSPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_queue_stats_prop_experimenter) == 12);

/* Body of reply to OFPMP_QUEUE_STATS request. */
struct ofp_queue_stats {
    big_uint16_t length;         /* Length of this entry. */
    big_uint8_t pad[6];          /* Align to 64 bits. */
    big_uint32_t port_no;        /* Port the queue is attached to. */
    big_uint32_t queue_id;       /* Queue i.d */
    big_uint64_t tx_bytes;       /* Number of transmitted bytes. */
    big_uint64_t tx_packets;     /* Number of transmitted packets. */
    big_uint64_t tx_errors;      /* Number of packets dropped due to overrun. */
    big_uint32_t duration_sec;   /* Time queue has been alive in seconds. */
    big_uint32_t duration_nsec;  /* Time queue has been alive in nanoseconds beyond
                                duration_sec. */

    struct ofp_queue_stats_prop_header properties[0]; /* List of properties. */
};
OFP_ASSERT(sizeof(struct ofp_queue_stats) == 48);

/* Body for ofp_multipart_request of type OFPMP_FLOW_MONITOR.
 *
 * The OFPMP_FLOW_MONITOR request's body consists of an array of zero or more
 * instances of this structure.  The request arranges to monitor the flows
 * that match the specified criteria, which are interpreted in the same way as
 * for OFPMP_FLOW.
 *
 * 'id' identifies a particular monitor for the purpose of allowing it to be
 * canceled later with OFPFMC_DELETE.  'id' must be unique among
 * existing monitors that have not already been canceled.
 */
struct ofp_flow_monitor_request {
    big_uint32_t monitor_id;        /* Controller-assigned ID for this monitor. */
    big_uint32_t out_port;          /* Required output port, if not OFPP_ANY. */
    big_uint32_t out_group;         /* Required group number, if not OFPG_ANY. */
    big_uint16_t flags;             /* OFPFMF_*. */
    big_uint8_t table_id;           /* One table's ID or OFPTT_ALL (all tables). */
    big_uint8_t command;            /* One of OFPFMC_*. */
    struct ofp_match match;     /* Fields to match. Variable size. */
};
OFP_ASSERT(sizeof(struct ofp_flow_monitor_request) == 24);

/* Flow monitor commands */
enum ofp_flow_monitor_command {
    OFPFMC_ADD    = 0,       /* New flow monitor. */
    OFPFMC_MODIFY = 1,       /* Modify existing flow monitor. */
    OFPFMC_DELETE = 2,       /* Delete/cancel existing flow monitor. */
};

/* 'flags' bits in struct of_flow_monitor_request. */
enum ofp_flow_monitor_flags {
    /* When to send updates. */
    OFPFMF_INITIAL = 1 << 0,     /* Initially matching flows. */
    OFPFMF_ADD = 1 << 1,         /* New matching flows as they are added. */
    OFPFMF_REMOVED = 1 << 2,     /* Old matching flows as they are removed. */
    OFPFMF_MODIFY = 1 << 3,      /* Matching flows as they are changed. */

    /* What to include in updates. */
    OFPFMF_INSTRUCTIONS = 1 << 4,/* If set, instructions are included. */
    OFPFMF_NO_ABBREV = 1 << 5,   /* If set, include own changes in full. */
    OFPFMF_ONLY_OWN = 1 << 6,    /* If set, don't include other controllers. */
};

/* OFPMP_FLOW_MONITOR reply header.
 *
 * The body of an OFPMP_FLOW_MONITOR reply is an array of variable-length
 * structures, each of which begins with this header.  The 'length' member may
 * be used to traverse the array, and the 'event' member may be used to
 * determine the particular structure.
 *
 * Every instance is a multiple of 8 bytes long. */
struct ofp_flow_update_header {
    big_uint16_t length;            /* Length of this entry. */
    big_uint16_t event;             /* One of OFPFME_*. */
    /* ...other data depending on 'event'... */
};
OFP_ASSERT(sizeof(struct ofp_flow_update_header) == 4);

/* 'event' values in struct ofp_flow_update_header. */
enum ofp_flow_update_event {
    /* struct ofp_flow_update_full. */
    OFPFME_INITIAL = 0,          /* Flow present when flow monitor created. */
    OFPFME_ADDED = 1,            /* Flow was added. */
    OFPFME_REMOVED = 2,          /* Flow was removed. */
    OFPFME_MODIFIED = 3,         /* Flow instructions were changed. */

    /* struct ofp_flow_update_abbrev. */
    OFPFME_ABBREV = 4,           /* Abbreviated reply. */

    /* struct ofp_flow_update_header. */
    OFPFME_PAUSED = 5,          /* Monitoring paused (out of buffer space). */
    OFPFME_RESUMED = 6,         /* Monitoring resumed. */
};

/* OFPMP_FLOW_MONITOR reply for OFPFME_INITIAL, OFPFME_ADDED, OFPFME_REMOVED,
 * and OFPFME_MODIFIED. */
struct ofp_flow_update_full {
    big_uint16_t length;            /* Length is 32 + match + instructions. */
    big_uint16_t event;             /* One of OFPFME_*. */
    big_uint8_t table_id;           /* ID of flow's table. */
    big_uint8_t reason;             /* OFPRR_* for OFPFME_REMOVED, else zero. */
    big_uint16_t idle_timeout;      /* Number of seconds idle before expiration. */
    big_uint16_t hard_timeout;      /* Number of seconds before expiration. */
    big_uint16_t priority;          /* Priority of the entry. */
    big_uint8_t zeros[4];           /* Reserved, currently zeroed. */
    big_uint64_t cookie;            /* Opaque controller-issued identifier. */
    struct ofp_match match;     /* Fields to match. Variable size. */
    /*     Instruction set.
     *     If OFPFMF_INSTRUCTIONS was not specified, or 'event' is
     *     OFPFME_REMOVED, no instructions are included.
     */
    //struct ofp_instruction instructions[0];
};
OFP_ASSERT(sizeof(struct ofp_flow_update_full) == 32);

/* OFPMP_FLOW_MONITOR reply for OFPFME_ABBREV.
 *
 * When the controller does not specify OFPFMF_NO_ABBREV in a monitor request,
 * any flow tables changes due to the controller's own requests (on the same
 * OpenFlow channel) will be abbreviated, when possible, to this form, which
 * simply specifies the 'xid' of the OpenFlow request (e.g. an OFPT_FLOW_MOD)
 * that caused the change.
 * Some changes cannot be abbreviated and will be sent in full.
 */
struct ofp_flow_update_abbrev {
    big_uint16_t length;            /* Length is 8. */
    big_uint16_t event;             /* OFPFME_ABBREV. */
    big_uint32_t xid;               /* Controller-specified xid from flow_mod. */
};
OFP_ASSERT(sizeof(struct ofp_flow_update_abbrev) == 8);

/* OFPMP_FLOW_MONITOR reply for OFPFME_PAUSED and OFPFME_RESUMED.
 */
struct ofp_flow_update_paused {
    big_uint16_t length;            /* Length is 8. */
    big_uint16_t event;             /* One of OFPFME_*. */
    big_uint8_t zeros[4];           /* Reserved, currently zeroed. */
};
OFP_ASSERT(sizeof(struct ofp_flow_update_paused) == 8);

/* Body for ofp_multipart_request/reply of type OFPMP_EXPERIMENTER. */
struct ofp_experimenter_multipart_header {
    big_uint32_t experimenter;    /* Experimenter ID. */
    big_uint32_t exp_type;        /* Experimenter defined. */
    /* Experimenter-defined arbitrary additional data. */
};
OFP_ASSERT(sizeof(struct ofp_experimenter_multipart_header) == 8);

/* Typical Experimenter structure. */
struct ofp_experimenter_structure {
    big_uint32_t experimenter;      /* Experimenter ID:
                                 * - MSB 0: low-order bytes are IEEE OUI.
                                 * - MSB != 0: defined by ONF. */
    big_uint32_t exp_type;          /* Experimenter defined. */
    big_uint8_t  experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_experimenter_structure) == 8);

/* Experimenter extension message. */
struct ofp_experimenter_msg {
    struct ofp_header header;   /* Type OFPT_EXPERIMENTER. */
    big_uint32_t experimenter;      /* Experimenter ID:
                                 * - MSB 0: low-order bytes are IEEE OUI.
                                 * - MSB != 0: defined by ONF. */
    big_uint32_t exp_type;          /* Experimenter defined. */
    /* Experimenter-defined arbitrary additional data. */
    big_uint8_t  experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_experimenter_msg) == 16);

/* Configures the "role" of the sending controller.  The default role is:
 *
 *    - Equal (OFPCR_ROLE_EQUAL), which allows the controller access to all
 *      OpenFlow features. All controllers have equal responsibility.
 *
 * The other possible roles are a related pair:
 *
 *    - Master (OFPCR_ROLE_MASTER) is equivalent to Equal, except that there
 *      may be at most one Master controller at a time: when a controller
 *      configures itself as Master, any existing Master is demoted to the
 *      Slave role.
 *
 *    - Slave (OFPCR_ROLE_SLAVE) allows the controller read-only access to
 *      OpenFlow features.  In particular attempts to modify the flow table
 *      will be rejected with an OFPBRC_EPERM error.
 *
 *      Slave controllers do not receive OFPT_PACKET_IN or OFPT_FLOW_REMOVED
 *      messages, but they do receive OFPT_PORT_STATUS messages.
 */

/* Controller roles. */
enum ofp_controller_role {
    OFPCR_ROLE_NOCHANGE = 0,    /* Don't change current role. */
    OFPCR_ROLE_EQUAL    = 1,    /* Default role, full access. */
    OFPCR_ROLE_MASTER   = 2,    /* Full access, at most one master. */
    OFPCR_ROLE_SLAVE    = 3,    /* Read-only access. */
};

/* Role request and reply message. */
struct ofp_role_request {
    struct ofp_header header;   /* Type OFPT_ROLE_REQUEST/OFPT_ROLE_REPLY. */
    big_uint32_t role;              /* One of OFPCR_ROLE_*. */
    big_uint8_t pad[4];             /* Align to 64 bits. */
    big_uint64_t generation_id;     /* Master Election Generation Id */
};
OFP_ASSERT(sizeof(struct ofp_role_request) == 24);

/* Role property types.
 */
enum ofp_role_prop_type {
    OFPRPT_EXPERIMENTER           = 0xFFFF, /* Experimenter property. */
};

/* Common header for all Role Properties */
struct ofp_role_prop_header {
    big_uint16_t         type;    /* One of OFPRPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_role_prop_header) == 4);

/* Experimenter role property */
struct ofp_role_prop_experimenter {
    big_uint16_t         type;    /* One of OFPRPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_role_prop_experimenter) == 12);

/* What changed about the controller role */
enum ofp_controller_role_reason {
    OFPCRR_MASTER_REQUEST = 0,  /* Another controller asked to be master. */
    OFPCRR_CONFIG         = 1,  /* Configuration changed on the switch. */
    OFPCRR_EXPERIMENTER   = 2,  /* Experimenter data changed. */
};

/* Role status event message. */
struct ofp_role_status {
    struct ofp_header header;   /* Type OFPT_ROLE_STATUS. */
    big_uint32_t role;              /* One of OFPCR_ROLE_*. */
    big_uint8_t reason;             /* One of OFPCRR_*. */
    big_uint8_t pad[3];             /* Align to 64 bits. */
    big_uint64_t generation_id;     /* Master Election Generation Id */

    /* Role Property list */
    struct ofp_role_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_role_status) == 24);

/* Async Config property types.
 * Low order bit cleared indicates a property for the slave role.
 * Low order bit set indicates a property for the master/equal role.
 */
enum ofp_async_config_prop_type {
    OFPACPT_PACKET_IN_SLAVE      = 0,  /* Packet-in mask for slave. */
    OFPACPT_PACKET_IN_MASTER     = 1,  /* Packet-in mask for master. */
    OFPACPT_PORT_STATUS_SLAVE    = 2,  /* Port-status mask for slave. */
    OFPACPT_PORT_STATUS_MASTER   = 3,  /* Port-status mask for master. */
    OFPACPT_FLOW_REMOVED_SLAVE   = 4,  /* Flow removed mask for slave. */
    OFPACPT_FLOW_REMOVED_MASTER  = 5,  /* Flow removed mask for master. */
    OFPACPT_ROLE_STATUS_SLAVE    = 6,  /* Role status mask for slave. */
    OFPACPT_ROLE_STATUS_MASTER   = 7,  /* Role status mask for master. */
    OFPACPT_TABLE_STATUS_SLAVE   = 8,  /* Table status mask for slave. */
    OFPACPT_TABLE_STATUS_MASTER  = 9,  /* Table status mask for master. */
    OFPACPT_REQUESTFORWARD_SLAVE  = 10, /* RequestForward mask for slave. */
    OFPACPT_REQUESTFORWARD_MASTER = 11, /* RequestForward mask for master. */
    OFPTFPT_EXPERIMENTER_SLAVE   = 0xFFFE, /* Experimenter for slave. */
    OFPTFPT_EXPERIMENTER_MASTER  = 0xFFFF, /* Experimenter for master. */
};

/* Common header for all async config Properties */
struct ofp_async_config_prop_header {
    big_uint16_t         type;    /* One of OFPACPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_async_config_prop_header) == 4);

/* Various reason based properties */
struct ofp_async_config_prop_reasons {
    big_uint16_t         type;    /* One of OFPACPT_PACKET_IN_*,
                                 OFPACPT_PORT_STATUS_*,
                                 OFPACPT_FLOW_REMOVED_*,
                                 OFPACPT_ROLE_STATUS_*,
                                 OFPACPT_TABLE_STATUS_*,
                                 OFPACPT_REQUESTFORWARD_*,
                                 OFPACPT_FLOW_STATS_*,
                                 OFPACPT_CONT_STATUS_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         mask;    /* Bitmasks of reason values. */
};
OFP_ASSERT(sizeof(struct ofp_async_config_prop_reasons) == 8);

/* Experimenter async config  property */
struct ofp_async_config_prop_experimenter {
    big_uint16_t         type;    /* One of OFPTFPT_EXPERIMENTER_SLAVE,
                                 OFPTFPT_EXPERIMENTER_MASTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_async_config_prop_experimenter) == 12);

/* Asynchronous message configuration. */
struct ofp_async_config {
    struct ofp_header header;     /* OFPT_GET_ASYNC_REPLY or OFPT_SET_ASYNC. */

    /* Async config Property list - 0 or more */
    struct ofp_async_config_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp_async_config) == 8);

/* What changed about the table */
enum ofp_table_reason {
    OFPTR_VACANCY_DOWN  = 3,        /* Vacancy down threshold event. */
    OFPTR_VACANCY_UP    = 4,        /* Vacancy up threshold event. */
};

/* A table config has changed in the datapath */
struct ofp_table_status {
    struct ofp_header header;
    big_uint8_t reason;         /* One of OFPTR_*. */
    big_uint8_t pad[7];         /* Pad to 64 bits */
    struct ofp_table_desc table;   /* New table config. */
};
OFP_ASSERT(sizeof(struct ofp_table_status) == 24);

/* Request forward reason */
enum ofp_requestforward_reason {
    OFPRFR_GROUP_MOD = 0,       /* Forward group mod requests. */
    OFPRFR_METER_MOD = 1,       /* Forward meter mod requests. */
};

/* Group/Meter request forwarding. */
struct ofp_requestforward_header {
    struct ofp_header header;   /* Type OFPT_REQUESTFORWARD. */
    struct ofp_header request;  /* Request being forwarded. */
};
OFP_ASSERT(sizeof(struct ofp_requestforward_header) == 16);

/* Bundle property types. */
enum ofp_bundle_prop_type {
    OFPBPT_EXPERIMENTER           = 0xFFFF, /* Experimenter property. */
};

/* Common header for all Bundle Properties */
struct ofp_bundle_prop_header {
    big_uint16_t         type;    /* One of OFPBPT_*. */
    big_uint16_t         length;  /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp_bundle_prop_header) == 4);

/* Experimenter bundle property */
struct ofp_bundle_prop_experimenter {
    big_uint16_t         type;    /* OFPBPT_EXPERIMENTER. */
    big_uint16_t         length;  /* Length in bytes of this property. */
    big_uint32_t         experimenter;  /* Experimenter ID which takes the same
                                       form as in struct
                                       ofp_experimenter_header. */
    big_uint32_t         exp_type;      /* Experimenter defined. */
    /* Followed by:
     *   - Exactly (length - 12) bytes containing the experimenter data, then
     *   - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
     *     bytes of all-zero bytes */
    big_uint32_t         experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp_bundle_prop_experimenter) == 12);

/* Bundle control message types */
enum ofp_bundle_ctrl_type {
    OFPBCT_OPEN_REQUEST    = 0,
    OFPBCT_OPEN_REPLY      = 1,
    OFPBCT_CLOSE_REQUEST   = 2,
    OFPBCT_CLOSE_REPLY     = 3,
    OFPBCT_COMMIT_REQUEST  = 4,
    OFPBCT_COMMIT_REPLY    = 5,
    OFPBCT_DISCARD_REQUEST = 6,
    OFPBCT_DISCARD_REPLY   = 7,
};

/* Bundle configuration flags. */
enum ofp_bundle_flags {
    OFPBF_ATOMIC  = 1 << 0,  /* Execute atomically. */
    OFPBF_ORDERED = 1 << 1,  /* Execute in specified order. */
};

/* Message structure for OFPT_BUNDLE_CONTROL. */
struct ofp_bundle_ctrl_msg {
    struct ofp_header     header;
    big_uint32_t              bundle_id;      /* Identify the bundle. */
    big_uint16_t              type;           /* OFPBCT_*. */
    big_uint16_t              flags;          /* Bitmap of OFPBF_* flags. */

    /* Bundle Property list. */
    struct ofp_bundle_prop_header properties[0]; /* Zero or more properties. */
};
OFP_ASSERT(sizeof(struct ofp_bundle_ctrl_msg) == 16);

/* Message structure for OFPT_BUNDLE_ADD_MESSAGE.
 * Adding a message in a bundle is done with. */
struct ofp_bundle_add_msg {
    struct ofp_header     header;
    big_uint32_t              bundle_id;      /* Identify the bundle. */
    big_uint16_t              pad;            /* Align to 64 bits. */
    big_uint16_t              flags;          /* Bitmap of OFPBF_* flags. */

    struct ofp_header     message;        /* Message added to the bundle. */

    /* If there is one property or more, 'message' is followed by:
     *   - Exactly (message.length + 7)/8*8 - (message.length) (between 0 and 7)
     *     bytes of all-zero bytes */

    /* Bundle Property list. */
    //struct ofp_bundle_prop_header properties[0]; /* Zero or more properties. */
};
OFP_ASSERT(sizeof(struct ofp_bundle_add_msg) == 24);

#endif /* openflow/openflow.h */
