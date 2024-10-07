<script lang="ts">
    import { interfaceProperties, saveWifiInterfaceProperties } from "./store";
    import { Button, Label, Modal, Input} from "./ui";

    interface Props {
        onClose: () => void;
    }

    let { onClose }: Props = $props();

    if (!interfaceProperties) {
        throw new Error("Interface properties is not loaded");
    }

    let hostname = $state($interfaceProperties.hostname);

    let saving = $state(false);
    let hostnameError: string | undefined = $state();
    let hostnameValid = $state(true);

    const save = async () => {
        hostnameError = undefined;
        saving = true;
        if (await saveWifiInterfaceProperties(hostname)) {
            onClose();
        } else {
            hostnameError = "Failed to save hostname";
        }
        saving = false;
    };
</script>

<Modal title="Interface Properties" size="xs" {onClose}>
    <Input
        label="Hostname"
        bind:value={hostname}
        help="Changes to hostname will be applied after the calendar has been reset."
        disabled={saving}
        status={{ error: !!hostnameError, message: hostnameError }}
        validator={async (value) => {
            hostnameValid = value.toString().match(/^[a-z0-9\-_]+$/i);
            return hostnameValid ? true : [false, "Enter a valid hostname"];
        }}
        maxlength={63}
        required
    />
    <Label class="space-y-1">
        <span>MAC address</span>
        <div class="font-normal text-sm text-gray-500">{$interfaceProperties.mac}</div>
    </Label>
    <div class="flex flex-row space-x-4">
        <Button disabled={saving} variant="outlined" class="grow" onclick={onClose}>Close</Button>
        <Button disabled={!hostnameValid} loading={saving} onclick={save} class="grow">Save</Button>
    </div>
</Modal>
