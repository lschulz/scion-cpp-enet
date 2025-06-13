from pathlib import Path
from datetime import datetime
from scapy_scion.layers.scion import UDP, SCION, EmptyPath
from scapy_scion.layers.scmp import *
from tests import write_packets


scion = SCION(
    qos = 32,
    fl = 0xf_ffff,
    dst_isd = 1,
    dst_asn = "ff00:0:1",
    src_isd = 2,
    src_asn = "ff00:0:2",
    dst_host = "10.0.0.1",
    src_host = "fd00::2",
    path = EmptyPath(),
)

payload = SCION(
    qos = 32,
    fl = 0xf_ffff,
    src_isd = 1,
    src_asn = "ff00:0:1",
    dst_isd = 2,
    dst_asn = "ff00:0:2",
    src_host = "10.0.0.1",
    dst_host = "fd00::2",
    path = EmptyPath(),
)
payload /= UDP(
    sport = 43000,
    dport = 1200,
    chksum = 0,
)
payload /= (1337).to_bytes(4, "big")


pkts = []

scmp = SCMP(
    code = 1,
    message = ScmpUnreachable()
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpPacketTooBig(mtu=1280)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 20,
    message = ScmpParameterProblem(pointer=8)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpExternalInterfaceDown(isd=3, asn="ff00:0:3", iface=20)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpInternalConnectivityDown(isd=3, asn="ff00:0:3", ingress=20, egress=30)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpEchoRequest(id=0xffff, seq=2)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpEchoReply(id=0xffff, seq=2)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpTracerouteRequest(id=0xffff, seq=2)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    code = 0,
    message = ScmpTracerouteReply(id=0xffff, seq=2, isd=3, asn="ff00:0:3", iface=20)
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    type = 100,
    code = 1,
)
pkts.append(bytes(scion / scmp / payload))

scmp = SCMP(
    type = 200,
    code = 1,
)
pkts.append(bytes(scion / scmp / payload))

write_packets(pkts, Path(__file__).with_suffix(".bin"))
