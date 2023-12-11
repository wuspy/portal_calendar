import fs from "fs";

const OUTPUT_PATH = "../resources/www";

export default function generateCppHeader(fileName: string, assetName: string) {
    const outputFilePath = `${OUTPUT_PATH}/${assetName}.h`;
    const assetCName = assetName.toLocaleUpperCase();
    console.log(`Generating C header for '${fileName}' at '${outputFilePath}'`);

    let output = [
        `/**`,
        ` * This is a generated source file.`,
        ` * Original file: ${fileName}`,
        ` * Build date: ${new Date()}`,
        ` */`,
        ``,
        `#ifndef ${assetCName}_H`,
        `#define ${assetCName}_H`,
        ``,
        `const uint8_t ${assetCName}_DATA[] = {\n`,
    ].join("\n");

    const buffer = Buffer.alloc(20);
    let offset = 0;
    let read = 0;
    const fd = fs.openSync(fileName, "r");
    while (read = fs.readSync(fd, buffer, 0, 20, offset)) {
        const bytes = [];
        for (let i = 0; i < read; ++i) {
            bytes.push(`0x${buffer[i].toString(16).padStart(2, "0")}`);
        }
        output += `    ${bytes.join()},\n`;
        offset += read;
    }
    fs.closeSync(fd);

    output += [
        `};`,
        ``,
        `#endif // ${assetCName}_H\n`
    ].join("\n");

    fs.writeFileSync(outputFilePath, output);
    console.log("Done");
}
