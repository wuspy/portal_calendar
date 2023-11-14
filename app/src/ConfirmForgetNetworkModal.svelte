<script lang="ts">
    import { forgetWifi, wifiStatus } from "./store";
    import { Button, Modal} from "./ui";
    import { createEventDispatcher } from "svelte";

    const dispatch = createEventDispatcher<{ close: void }>();

    let saving = false;

    const forget = async () => {
        saving = true;
        if (await forgetWifi()) {
            dispatch("close");
        }
        saving = false;
    };
</script>

<Modal title="Forget network" size="xs" on:close>
    <p class="text-md">
        Are you sure you want to forget the network '{$wifiStatus.ssid}'?
        You will need to connect to another network before the calendar can be used again.
    </p>
    <div class="flex flex-row space-x-4">
        <Button disabled={saving} variant="outlined" class="grow" on:click={() => dispatch("close")}>Cancel</Button>
        <Button loading={saving} on:click={forget} class="grow">Forget</Button>
    </div>
</Modal>
