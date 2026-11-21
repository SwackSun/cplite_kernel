# SPDX-License-Identifier: MIT
from pathlib import Path
SOURCE = Path(__file__).with_name("cardputerzero.splash")

def panel_firmware():
    """Compile the shared readable DCS setup into Linux's MIPI DBI format."""
    result = bytearray(b'MIPI DBI\0\0\0\0\0\0\0\x01')
    for line in SOURCE.read_text(encoding='utf-8').splitlines():
        words = line.split('#', 1)[0].split()
        if words[:2] == ['delay', '[ms]']:
            delay = int(words[2], 0)
            if not 1 <= delay <= 255:
                raise ValueError('Linux panel delay must fit one byte')
            result.extend((0, 1, delay))
        elif words[:1] == ['panel']:
            command, *parameters = [int(word, 0) for word in words[1:]]
            if command in (0x2a, 0x2b):
                continue
            result.extend((command, len(parameters), *parameters))
    return bytes(result)

(SOURCE.parents[1] / "linux/cm0-firmware/cardputerzero,st7789v_lcd.bin").write_bytes(panel_firmware())
