from PIL import Image

pixelMap: dict[int, int] = {}

for x in range(0, 43): pixelMap[x] = 0b11
for x in range(43, 128): pixelMap[x] = 0b10
for x in range(128, 213): pixelMap[x] = 0b01
for x in range(213, 256): pixelMap[x] = 0b00

OUTPUT_BYTES_PER_LINE = 20

def compileImage(image: Image) -> tuple[list[str], int]:
    image = image.convert(mode="L", dither=Image.NONE)
    outputLines: list[str] = [""]
    currentBytesOnLine = 0
    currentByte = 0
    currentBit = 8
    byteCount = 0

    for px in image.getdata():
        currentBit -= 2
        currentByte |= pixelMap[px] << currentBit
        if currentBit == 0:
            if currentBytesOnLine == OUTPUT_BYTES_PER_LINE:
                outputLines.append("")
                currentBytesOnLine = 0
            outputLines[-1] += "0x{:02X},".format(currentByte)
            byteCount += 1
            currentBytesOnLine += 1
            currentBit = 8
            currentByte = 0

    if currentBit != 8: # In case width * height is not a multiple of 4
        byteCount += 1
        outputLines[-1] += "0x{:02X},".format(currentByte)
    
    return outputLines, byteCount
