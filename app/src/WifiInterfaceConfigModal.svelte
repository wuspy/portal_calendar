<script lang="ts">
    import { interfaceProperties, saveWifiInterfaceProperties } from "./store";
    import { Button, Label, Modal, Input} from "./ui";
    import { createEventDispatcher } from "svelte";

    const dispatch = createEventDispatcher<{ close: void }>();

    let { hostname, mac } = $interfaceProperties;
    let saving = false;
    let hostnameError: string | undefined = undefined;
    let hostnameValid: boolean = true;

    const save = async () => {
        hostnameError = undefined;
        saving = true;
        if (await saveWifiInterfaceProperties(hostname)) {
            dispatch("close");
        } else {
            hostnameError = "Failed to save hostname";
        }
        saving = false;
    };
</script>

<Modal title="Interface Properties" size="xs" on:close>
    <Input
        label="Hostname"
        bind:value={hostname}
        help="Changes to hostname will be applied after the calendar has been reset"
        disabled={saving}
        status={{ error: !!hostnameError, message: hostnameError }}
        validator={async (value) => {
            hostnameValid = value.toString().match(/^[a-z0-9\-_]+$/i);
            return hostnameValid ? true : [false, "Enter a valid hostname"];
        }}
        maxlength=63
        required
    />
    <Label class="space-y-1">
        <span>MAC address</span>
        <div class="font-normal text-sm text-gray-500">{mac}</div>
    </Label>
    <div class="flex flex-row space-x-4">
        <Button disabled={saving} variant="outlined" class="grow" on:click={() => dispatch("close")}>Close</Button>
        <Button disabled={!hostnameValid} loading={saving} on:click={save} class="grow">Save</Button>
    </div>
</Modal>
