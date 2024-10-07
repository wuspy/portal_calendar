<script context="module" lang="ts">
    // Caches working servers to prevent unnecessary requests to validate servers we already know to work.
    // Many NTP servers (including NIST) have rate limiting, as does rop's timezoned code, so this also
    // prevents the user from sending multiple requests quickly to the same server.
    const validNtpServers: string[] = [];
    const validTimezonedServers: string[] = [];

    const browserTimezone = Intl.DateTimeFormat().resolvedOptions().timeZone ?? "";
    const timezoneList: SelectOptionType[] | undefined =
        typeof Intl.supportedValuesOf === "function"
            ? Intl.supportedValuesOf("timeZone").map(timezone => ({
                  value: timezone,
                  name: timezone,
              }))
            : undefined;
</script>

<script lang="ts">
    import { fly } from "svelte/transition";
    import Api from "./api";
    import { openSaveFailedModal } from "./SaveFailedModal.svelte";
    import { preferences, savePreferences, locales } from "./store";
    import {
        Checkbox,
        Button,
        Input,
        Select,
        SvgIcon,
        Label,
        type SelectOptionType,
        type InputValidator,
        WizardPageLayout,
        type InputStatus,
    } from "./ui";

    interface Props {
        onBack: () => void;
        onNext: () => void;
    }

    let { onBack, onNext }: Props = $props();

    let timezone = $state($preferences.timezone || browserTimezone);
    let locale = $state($preferences.locale);
    let twoNtpSyncs = $state($preferences.twoNtpSyncs);
    let rtcCorrection = $state($preferences.rtcCorrection);
    let ntpServer1 = $state($preferences.ntpServer1);
    let ntpServer2 = $state($preferences.ntpServer2);
    let tzdServer1 = $state($preferences.tzdServer1);
    let tzdServer2 = $state($preferences.tzdServer2);
    let showDay = $state($preferences.showDay);
    let showMonth = $state($preferences.showMonth);
    let showYear = $state($preferences.showYear);

    let showAdvanced = $state(false);
    let rtcStatus: InputStatus = $state({});
    let primaryNtpServerStatus: InputStatus = $state({});
    let primaryTimezonedServerStatus: InputStatus = $state({});
    let saving = $state(false);

    const checkNtpServer: InputValidator = async (server: string) => {
        server = server.trim();
        if (!server) {
            return true;
        }
        if (validNtpServers.includes(server)) {
            return [true, "Server is working"];
        }
        try {
            const result = await Api.testNtpServer(server);
            switch (result.status) {
                case 200:
                    validNtpServers.push(server);
                    return [true, "Server is working"];
                case 502:
                    return [false, "Server is not working"];
                default:
                    throw result.status;
            }
        } catch (error) {
            return [false, "Error communicating with calendar"];
        }
    };

    const checkTzdServer: InputValidator = async (server: string) => {
        server = server.trim();
        if (!server) {
            return true;
        }
        if (validTimezonedServers.includes(server)) {
            return [true, "Server is working"];
        }
        try {
            const result = await Api.lookupTimezone(server, timezone);
            switch (result.status) {
                case 200:
                    console.info(`Timezone for ${timezone}: ${await result.text()}`);
                case 404: // Location not found still means the server works
                    validTimezonedServers.push(server);
                    return [true, "Server is working"];
                case 502:
                    return [false, "Server is not working"];
                default:
                    throw result.status;
            }
        } catch (error) {
            return [false, "Error communicating with calendar"];
        }
    };

    const checkRtcCorrectionFactor: InputValidator = async (value: number) =>
        value > 1 || value < 0
            ? [false, "Enter a number between 0 and 1"]
            : true;

    async function saveAndContinue() {
        saving = true;
        if (await savePreferences({
            timezone,
            locale,
            twoNtpSyncs,
            rtcCorrection,
            ntpServer1,
            ntpServer2,
            tzdServer1,
            tzdServer2,
            showDay,
            showMonth,
            showYear,
        })) {
            onNext();
        } else {
            openSaveFailedModal();
            saving = false;
        }
    }
</script>

<WizardPageLayout
    title="Date & Time"
    {onBack}
    onNext={saveAndContinue}
    {saving}
    nextDisabled={!timezone || primaryNtpServerStatus.error || primaryTimezonedServerStatus.error || rtcStatus.error}
>
    {#if timezoneList}
        <Select
            label="Timezone"
            bind:value={timezone}
            items={timezoneList}
        />
    {:else}
        <Input
            label="Timezone"
            bind:value={timezone}
        />
    {/if}
    <Select
        label="Language"
        bind:value={locale}
        items={Object.entries($locales).map(([code, name]) => ({ value: code, name }))}
        help="Select the language for the day & month names. This setup wizard is currently only available in English."
    />
    <Checkbox
        label="Show Day"
        bind:checked={showDay}
        help="Show the current day on the right"
    />
    <Checkbox
        label="Show Month"
        bind:checked={showMonth}
        help="Show the current month on the top left"
    />
    <Checkbox
        label="Show Year"
        bind:checked={showYear}
        help="Show the current year on the top right"
    />
    <div class="flex">
        <Button
            variant="text"
            class="-mb-4 grow"
            onclick={() => (showAdvanced = !showAdvanced)}
        >
            {#if showAdvanced}
                <SvgIcon icon="ExpandLess" size="md" />
                <span class="mx-2">Hide Advanced Options</span>
            {:else}
                <SvgIcon icon="ExpandMore" size="md" />
                <span class="mx-2">Show Advanced Options</span>
            {/if}
        </Button>
    </div>
    {#if showAdvanced}
        <div class="border-t"></div>
        <div class="space-y-6" in:fly={{ y: -20, duration: 200 }}>
            <Checkbox
                label="Perform two NTP syncs per day"
                bind:checked={twoNtpSyncs}
                help="This will ensure the date changeover happens exactly at midnight, but shortens battery life."
            />
            <Label class="space-y-2">
                <span>NTP Servers</span>
                <Input
                    bind:value={ntpServer1}
                    bind:status={primaryNtpServerStatus}
                    validator={checkNtpServer}
                    validateOnMount
                    placeholder="Primary NTP server"
                    required
                >
                    {#snippet left()}
                        <span class="mx-1">1.</span>
                    {/snippet}
                </Input>
                <Input
                    bind:value={ntpServer2}
                    validator={checkNtpServer}
                    validateOnMount
                    placeholder="Secondary NTP server"
                >
                    {#snippet left()}
                        <span class="mx-1">2.</span>
                    {/snippet}
                </Input>
            </Label>
            <Label class="space-y-2">
                <span>Timezoned Servers</span>
                <Input
                    bind:value={tzdServer1}
                    bind:status={primaryTimezonedServerStatus}
                    validator={checkTzdServer}
                    validateOnMount
                    placeholder="Primary timezoned server"
                    required
                >
                    {#snippet left()}
                        <span class="mx-1">1.</span>
                    {/snippet}
                </Input>
                <Input
                    bind:value={tzdServer2}
                    validator={checkTzdServer}
                    validateOnMount
                    placeholder="Secondary timezoned server"
                >
                    {#snippet left()}
                        <span class="mx-1">2.</span>
                    {/snippet}
                </Input>
            </Label>
            <Input
                label="Max RTC correction factor"
                type="number"
                bind:value={rtcCorrection}
                bind:status={rtcStatus}
                min="0"
                max="1"
                step="0.001"
                validator={checkRtcCorrectionFactor}
                required
                help={"The calendar measures drift in the system clock on each NTP measurement and calculates a correction factor to improve timekeeping."
                    + " This controls the maximum correction factor that is allowed. The default is 0.025."
                    + " To disable this feature you can just set this to zero."}
            />
        </div>
    {/if}
</WizardPageLayout>
