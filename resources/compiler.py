from PIL import Image
from math import ceil, pow

pixelMap: dict[int, int] = {}

for x in range(0, 43): pixelMap[x] = 0b11
for x in range(43, 128): pixelMap[x] = 0b10
for x in range(128, 213): pixelMap[x] = 0b01
for x in range(213, 256): pixelMap[x] = 0b00

OUTPUT_BYTES_PER_LINE = 20

def compileImage(image: Image) -> tuple[list[str], int, int, int]:
    image = image.convert(mode="L", dither=Image.NONE)

    # Do a first pass to count the quantities of each consecutive pixel run, and use that
    # data to calculate the optimum number of bits for RLE

    rleCounts: dict[int, int] = {}
    currentRun = 0
    currentColor = 0
    for px in image.getdata():
        color = pixelMap[px]
        if color == currentColor:
            currentRun += 1
        else:
            rleCounts[currentRun] = rleCounts[currentRun] + 1 if currentRun in rleCounts else 1
            currentRun = 1
            currentColor = color

    rleBits = 0
    # Uncompressed native size of the image in 2bpp, which may actually
    # be smaller for certain inputs
    rleSize = uncompressedSize = image.size[0] * image.size[1] * 2
    # Only even numbers of bits are considered for RLE because they allow the image to be encoded/decoded
    # in 2-bit chunks and ensure that at least the color value isn't split across bytes
    for bits in [2, 4, 6]:
        size = sum([
            ceil(run / pow(2, bits)) * (2 + bits) * count for run, count in rleCounts.items()
        ])
        if size < rleSize:
            rleSize = size
            rleBits = bits

    # Now encode the image

    outputLines: list[str] = [""]
    currentBytesOnLine = 0
    currentByte = 0
    currentCrumb = 0
    byteCount = 0

    def appendCrumb(crumb: int):
        nonlocal currentByte, currentCrumb, currentBytesOnLine, outputLines, byteCount
        currentByte |= (crumb & 0b11) << ((3 - currentCrumb) * 2)
        if currentCrumb == 3:
            if currentBytesOnLine == OUTPUT_BYTES_PER_LINE:
                outputLines.append("")
                currentBytesOnLine = 0
            outputLines[-1] += "0x{:02X},".format(currentByte)
            byteCount += 1
            currentBytesOnLine += 1
            currentCrumb = 0
            currentByte = 0
        else:
            currentCrumb += 1

    def appendRun(color: int, run: int):
        nonlocal rleBits
        if run >= 0:
            appendCrumb(color)
            for i in reversed(range(0, rleBits - 1, 2)):
                appendCrumb(run >> i)

    currentRun = -1
    currentColor = 0
    rleMaxRun = pow(2, rleBits) - 1
    for px in image.getdata():
        color = pixelMap[px]
        if color == currentColor and currentRun < rleMaxRun:
            currentRun += 1
        else:
            appendRun(currentColor, currentRun)
            currentRun = 0
            currentColor = color

    appendRun(currentColor, currentRun)
    if currentCrumb != 0: # Write remaining partial byte if needed
        byteCount += 1
        outputLines[-1] += "0x{:02X},".format(currentByte)
    
    return outputLines, byteCount, uncompressedSize / 8 / byteCount if byteCount > 0 else 0, rleBits
