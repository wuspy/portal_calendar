import argparse
from compiler import compileImage, pixelMap
from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont
from os import path

class Glyph:
    char: str
    codePoint: int
    index: int
    width: int
    height: int
    top: int
    left: int

    def __init__(self, index: int, char: str, codePoint: int, font: ImageFont):
        self.char = char
        self.codePoint = codePoint
        self.index = index
        self.left, self.top, right, bottom = font.getbbox(char)
        self.width = right - self.left + 1
        self.height = bottom - self.top + 1

    def empty(self) -> bool:
        return self.width * self.height == 0

    def size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def compile(self, data: str) -> str:
        return "{{ 0x{0.codePoint:04X}, {{ .width={0.width}, .height={0.height}, .top={0.top}, .left={0.left}, .data=&_{1}_DATA[{0.index}] }} }}, // '{0.char}'".format(self, data)

parser = argparse.ArgumentParser()
parser.add_argument("font", type=str, help="Name or path of the font to be compiled")
parser.add_argument("-name", type=str, required=True, help="Name for the generated bitmap font")
parser.add_argument("-size", type=int, required=True, help="The size in pixels of the compiled bitmap font")
parser.add_argument("-ranges", type=str, required=True, help="The Unicode character ranges to include in the bitmap font (i.e. 0-9,A-Z,À-ÿ)")
parser.add_argument("-fg", type=int, required=False, default=0, help="The foreground color, 0-255, defaults to 0 (black)")
parser.add_argument("-bg", type=int, required=False, default=255, help="The background color, 0-255, defaults to 255 (white)")
args = parser.parse_args()

codePointRanges = list(map(lambda range : range.split('-', 1), args.ranges.split(',')))
fontSize = args.size
fgColor = args.fg
bgColor = args.bg
fontPath = args.font  # '/dir/my-font.ttf'
fontName = args.name # 'output-font-name'
outputFileName = "{}/{}.h".format(path.dirname(fontPath), fontName) # '/dir/output-font-name.h'
fontCName = "FONT_{}".format(fontName.upper().replace(".", "_").replace("-", "_").replace(" ", "_")) # 'FONT_OUTPUT_FONT_NAME'

print("Building font '{}' at size {}px with ranges {}".format(fontPath, fontSize, codePointRanges))

font = ImageFont.truetype(fontPath, fontSize)
ascent, descent = font.getmetrics()

fontTables = list(map(lambda table : table.cmap.keys(), TTFont(fontPath)['cmap'].tables))
def fontHasCodePoint(codePoint: int) -> bool:
    for table in fontTables:
        if codePoint in table:
            return True
    return False

outputLines: list[str] = []
outputGlyphs: list[Glyph] = []
byteCount = 0
for codePointRange in codePointRanges:
    for codePoint in range(ord(codePointRange[0]), ord(codePointRange[1] if len(codePointRange) > 1 else codePointRange[0]) + 1):
        char = chr(codePoint)
        glyph = Glyph(byteCount, char, codePoint, font)
        if not fontHasCodePoint(codePoint) and codePoint != 0xFFFD:
            print("Warning: Font does not contain code point '{}' (U+{:04X})".format(char, codePoint))
        elif glyph.empty():
            print("Skipping whitespace char U+{:04X}".format(codePoint))
        else:
            outputGlyphs.append(glyph)
            image = Image.new("L", glyph.size(), bgColor)
            draw = ImageDraw.Draw(image)
            draw.text((-glyph.left, -glyph.top), char, font=font, fill=fgColor)
            charOutputLines, charByteCount = compileImage(image)
            outputLines.append("// '{}'".format(char))
            outputLines += charOutputLines
            byteCount += charByteCount

print("Output size: {}".format(byteCount))
print("Writing to file '{}'".format(outputFileName))

outputFile = open(outputFileName, mode="w", encoding="utf8")
outputFile.writelines([
    "/**\n",
    " * This is a generated source file.\n",
    " * Original font: {}\n".format(fontPath),
    " * Font size: {}px\n".format(fontSize),
    " * Code point ranges: {}\n".format(args.ranges),
    " */\n\n",

    "#include \"font.h\"\n\n",

    "#ifndef {}_H\n".format(fontCName),
    "#define {}_H\n\n".format(fontCName),

    "const uint8_t _{}_DATA[] = {{\n".format(fontCName),
        *map(lambda x : "    {}\n".format(x), outputLines),
    "};\n\n",
    
    "const Font {} {{\n".format(fontCName),
    "    .glyphs={\n",
            *map(lambda glyph : "        {}\n".format(glyph.compile(fontCName)), outputGlyphs),
    "    },\n",
    "    .fgColor=0b{:02b},\n".format(pixelMap[fgColor]),
    "    .bgColor=0b{:02b},\n".format(pixelMap[bgColor]),
    "    .ascent={},\n".format(ascent),
    "    .descent={},\n".format(descent),
    "    .spaceWidth={},\n".format(round(font.getlength(" "))),
    "};\n\n",

    "#endif // {}_H\n".format(fontCName),
])
outputFile.close()

print("Done")
