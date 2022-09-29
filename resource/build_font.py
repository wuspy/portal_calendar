import argparse
from compiler import compileImage, pixelMap
from PIL import Image, ImageDraw, ImageFont
import os.path as path

class Glyph:
    char: str
    index: int
    width: int
    height: int
    top: int
    left: int

    def __init__(self, index, char: str, font: ImageFont):
        self.char = char
        self.index = index
        self.left, self.top, right, bottom = font.getbbox(char)
        self.width = right - self.left + 1
        self.height = bottom - self.top + 1

    def empty(self) -> bool:
        return self.width * self.height == 0

    def size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def compile(self, data: str) -> str:
        return "{{ .width={0.width}, .height={0.height}, .top={0.top}, .left={0.left}, .data=&_{1}_DATA[{0.index}] }}, // '{0.char}'".format(self, data)

parser = argparse.ArgumentParser()
parser.add_argument("font", type=str, help="Name or path of the font to be compiled")
parser.add_argument("size", type=int, help="The size in pixels of the compiled bitmap font")
parser.add_argument("name", type=str, help="Name for the generated bitmap font")
parser.add_argument("-start", type=str, required=False, default='!', help="The beginning of the range of ASCII characters to include")
parser.add_argument("-end", type=str, required=False, default='~', help="The end of the range of ASCII characters to include")
parser.add_argument("-fg", type=int, required=False, default=0, help="The foreground color, 0-255, defaults to 0 (black)")
parser.add_argument("-bg", type=int, required=False, default=255, help="The background color, 0-255, defaults to 255 (white)")
args = parser.parse_args()

asciiStart = min(ord(args.start), ord(args.end))
asciiEnd = max(ord(args.start), ord(args.end))

chars = map(lambda char: chr(char), range(asciiStart, asciiEnd + 1))

fontSize = args.size
fgColor = args.fg
bgColor = args.bg
fontPath = args.font  # '/dir/my-font.ttf'
fontName = args.name # 'output-font-name'
outputFileName = "{}/{}.h".format(path.dirname(fontPath), fontName) # '/dir/output-font-name.h'
fontCName = "FONT_{}".format(fontName.upper().replace(".", "_").replace("-", "_").replace(" ", "_")) # 'FONT_OUTPUT_FONT_NAME'

print("Building font '{0}'".format(fontPath))

font = ImageFont.truetype(fontPath, fontSize)
ascent, descent = font.getmetrics()

outputLines: list[str] = []
outputGlyphs: list[Glyph] = []
byteCount = 0
for char in chars:
    glyph = Glyph(byteCount, char, font)
    if glyph.empty():
        print("Skipping whitespace char 0x{:02X}".format(ord(char)))
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

outputFile = open(outputFileName, "w")
outputFile.writelines([
    "/**\n",
    " * This is a generated source file.\n",
    " * Original font: {}\n".format(fontPath),
    " * Font size: {}px\n".format(fontSize),
    " */\n\n",

    "#include \"font.h\"\n\n",

    "#ifndef {}_H\n".format(fontCName),
    "#define {}_H\n\n".format(fontCName),

    "const uint8_t _{}_DATA[] = {{\n".format(fontCName),
])
outputFile.writelines(map(lambda x : "    {}\n".format(x), outputLines))
outputFile.writelines([
    "};\n\n",
    
    "const FontGlyph _{}_GLYPHS[] = {{\n".format(fontCName),
])
outputFile.writelines(map(lambda glyph : "    {}\n".format(glyph.compile(fontCName)), outputGlyphs))
outputFile.writelines([
    "};\n\n",
    
    "const Font {} {{\n".format(fontCName),
    "    .rangeStart=0x{:02X}, // {}\n".format(asciiStart, chr(asciiStart)),
    "    .rangeEnd=0x{:02X}, // {}\n".format(asciiEnd, chr(asciiEnd)),
    "    .fgColor=0b{:02b},\n".format(pixelMap[fgColor]),
    "    .bgColor=0b{:02b},\n".format(pixelMap[bgColor]),
    "    .ascent={},\n".format(ascent),
    "    .descent={},\n".format(descent),
    "    .spaceWidth={},\n".format(round(font.getlength(" "))),
    "    .glyphs=_{}_GLYPHS,\n".format(fontCName),
    "};\n\n",

    "#endif // {}_H\n".format(fontCName),
])
outputFile.close()

print("Done")
