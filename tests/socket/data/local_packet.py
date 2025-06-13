from pathlib import Path
from scapy_scion.layers.scion import UDP, SCION, EmptyPath
from tests import write_packets


scion = SCION(
    qos = 0,
    dst_isd = 1,
    dst_asn = "ff00:0:1",
    src_isd = 1,
    src_asn = "ff00:0:1",
    dst_host = "fd00::1",
    src_host = "10.0.0.1",
    path = EmptyPath(),
)

udp = UDP(sport=3000, dport=8000)

payload = b"\x01\x02\x03\x04\x05\x06\x07\x08"

pkt = scion / udp / payload

write_packets([pkt], Path(__file__).with_suffix(".bin"))
