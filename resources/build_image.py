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

outputLines, byteCount = compileImage(image)

print("Output size: {}".format(byteCount))
print("Writing to file '{}'".format(outputFileName))

outputFile = open(outputFileName, mode="w", encoding="utf8")
outputFile.writelines([
    "/**\n",
    " * This is a generated source file.\n",
    " * Original image: {}\n".format(imagePath),
    " */\n\n",

    "#include \"image.h\"\n\n",

    "#ifndef {}_H\n".format(imageCName),
    "#define {}_H\n\n".format(imageCName),

    "const uint8_t _{}_DATA[] = {{\n".format(imageCName),
        *map(lambda line : "    {}\n".format(line), outputLines),
    "};\n\n",

    "const Image {} {{\n".format(imageCName),
    "    .width={},\n".format(width),
    "    .height={},\n".format(height),
    "    .data=_{}_DATA,\n".format(imageCName),
    "};\n\n",

    "#endif // {}_H\n".format(imageCName),
])
outputFile.close()

print("Done")
