/// <reference types="svelte" />
/// <reference types="vite/client" />

namespace Intl {
    /**
     * Limited browser support.
     * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/supportedValuesOf
     */
    declare function supportedValuesOf(key: string): string[];
}
