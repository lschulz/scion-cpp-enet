from pathlib import Path
from scapy.layers.l2 import Ether
from scapy.layers.inet6 import IPv6, UDP
from tests import write_packets


pkt = Ether(
    dst = "00:00:00:00:00:01",
    src = "00:00:00:00:00:02"
)

pkt /= IPv6(
    tc = 32,
    fl = 0xf_ffff,
    plen = 12,
    hlim = 20,
    src = "fd00::1",
    dst = "fd00::2"
)

pkt /= UDP(
    sport = 43000,
    dport = 1200,
)

pkt /= (1337).to_bytes(4, "big")

write_packets([pkt], Path(__file__).with_suffix(".bin"))
