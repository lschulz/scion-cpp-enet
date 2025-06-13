from pathlib import Path
from scapy.layers.inet6 import IPv6, ICMPv6EchoReply
from tests import write_packets

pkt = IPv6(
    tc = 32,
    fl = 0xf_ffff,
    hlim = 20,
    src = "fd00::1",
    dst = "fd00::2"
)

pkt /= ICMPv6EchoReply(
    id = 1337,
    seq = 1,
)

write_packets([pkt], Path(__file__).with_suffix(".bin"))
