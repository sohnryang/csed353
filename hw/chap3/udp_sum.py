words = [
    0xC0A8,
    0x0107,
    0x8DDF,
    0x054E,
    0x0011,
    0x000C,
    0xC5E8,
    0xC351,
    0x000C,
    0x5CF8,
    0x6162,
    0x6364,
]
checksum = 0
for word in words:
    checksum += word
    carryover = checksum >> 16
    checksum = (checksum & 0xFFFF) + carryover
print(f"checksum: {checksum:04x}")
