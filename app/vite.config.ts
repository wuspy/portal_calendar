import { svelte } from "@sveltejs/vite-plugin-svelte";
import sveltePreprocess from "svelte-preprocess";
import { visualizer } from "rollup-plugin-visualizer";
import { defineConfig } from "vite";
import viteCompression from "vite-plugin-compression";
import { viteSingleFile } from "vite-plugin-singlefile"
import generateCppHeader from "./scripts/generateCppHeader";

export default defineConfig({
    plugins: [
        svelte({
            preprocess: [sveltePreprocess({ typescript: true })],
        }),
        visualizer({
            gzipSize: true,
        }),
        viteSingleFile(),
        viteCompression({
            algorithm: "gzip",
            success: () => generateCppHeader("dist/index.html.gz", "index_html"),
        }),
    ],
});
