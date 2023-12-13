import argparse
from compiler import compileImage
from PIL import Image
import os.path as path

parser = argparse.ArgumentParser()
parser.add_argument("path", type=str, help="Path of the image to be compiled")
args = parser.parse_args()

imagePath = args.path # '/dir/foo-bar.gif'
imageName = path.basename(path.splitext(imagePath)[0]) # 'foo-bar'
outputFileName = "{}.h".format(path.splitext(imagePath)[0]) # '/dir/foo-bar.h'
imageCName = "IMG_{}".format(imageName.upper().replace(".", "_").replace("-", "_").replace(" ", "_")) # 'IMG_FOO_BAR'

image = Image.open(imagePath)
print("Loaded file '{}'".format(imagePath))

width, height = image.size

print(" - Compiling image...")
outputLines, byteCount, compressionRatio, rleBits = compileImage(image)
print(" - Optimum RLE bits: {}".format(rleBits))
print(" - Output size: {} bytes ({} compression ratio)".format(byteCount, round(compressionRatio, 2)))
print(" - Writing to file '{}'...".format(outputFileName))

outputFile = open(outputFileName, mode="w", encoding="utf8")
outputFile.writelines([
    "/**\n",
    " * This is a generated source file.\n",
    " * Original image: {}\n".format(imagePath),
    " * Compression ratio: {}\n".format(round(compressionRatio, 2)),
    " */\n\n",

    "#include \"image.h\"\n\n",

    "#ifndef {}_H\n".format(imageCName),
    "#define {}_H\n\n".format(imageCName),

    "const uint8_t _{}_DATA[] = {{\n".format(imageCName),
        *map(lambda line : "    {}\n".format(line), outputLines),
    "};\n\n",

    "const Image {0} = Image({1}, {2}, {3}, _{0}_DATA);\n\n".format(imageCName, width, height, rleBits),

    "#endif // {}_H\n".format(imageCName),
])
outputFile.close()

print(" - Done")
