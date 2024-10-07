<script lang="ts">
    import { forgetWifi, wifiStatus } from "./store";
    import { Button, Modal} from "./ui";

    interface Props {
        onClose: () => void;
    }

    let { onClose }: Props = $props();

    let saving = $state(false);

    const forget = async () => {
        saving = true;
        if (await forgetWifi()) {
            onClose();
        }
        saving = false;
    };
</script>

<Modal title="Forget network" size="xs" {onClose}>
    <p class="text-md">
        Are you sure you want to forget the network '{$wifiStatus.ssid}'?
        You will need to connect to another network before the calendar can be used again.
    </p>
    <div class="flex flex-row space-x-4">
        <Button disabled={saving} variant="outlined" class="grow" onclick={onClose}>Cancel</Button>
        <Button loading={saving} onclick={forget} class="grow">Forget</Button>
    </div>
</Modal>
