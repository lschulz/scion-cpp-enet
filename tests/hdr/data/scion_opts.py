from pathlib import Path
from scapy_scion.layers.scion import (
    UDP, HopByHopExt, EndToEndExt, Pad1Option, PadNOption, AuthenticatorOption
)
from tests import write_packets


pkt = HopByHopExt(options=[
    Pad1Option(),
    Pad1Option(),
    PadNOption(data=2*b"\x00"),
])

pkt /= EndToEndExt(options=[
    AuthenticatorOption(
        spi=0xffff,
        algorithm="AES-CMAC",
        timestamp=1,
        authenticator=b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    ),
])

pkt /= UDP()

write_packets([pkt], Path(__file__).with_suffix(".bin"))
