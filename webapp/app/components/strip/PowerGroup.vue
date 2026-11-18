<script setup>
import { computed, onBeforeUnmount } from 'vue';
import DashboardSpeed01Icon from '@hugeicons/core-free-icons/DashboardSpeed01Icon';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Progress } from '@brumaombra/ui-vintage/progress';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { createSettingsSaver } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const POWER_LIMIT_STEP = 100; // Milliamps
const { t } = useI18n();
const globalStore = useGlobalStore();
const saveMaxMilliamps = createSettingsSaver();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Whether the estimated current is over the power limit (the device then dims the LEDs)
const isPowerLimited = computed(() => device.value.on && device.value.estimatedMilliamps >= device.value.maxMilliamps);

// Change the power limit (the device gets one request when the slider rests)
const handleMaxMilliampsChange = ([maxMilliamps]) => {
    saveMaxMilliamps({ maxMilliamps });
};

// Send a power limit still waiting when leaving the page
onBeforeUnmount(() => saveMaxMilliamps.flush());
</script>

<template>
    <div class="space-y-3">
        <!-- The power: the current drawn by the LEDs against the limit, and the limit -->
        <ListGroup :title="t('pages.strip.powerTitle')" :hint="t('pages.strip.maxMilliamps.note')">
            <!-- Current drawn now -->
            <ListRow :label="t('pages.strip.current.label')" :icon="DashboardSpeed01Icon" :value="t('pages.strip.current.value', { milliamps: device.estimatedMilliamps, limit: device.maxMilliamps })">
                <Progress :model-value="device.estimatedMilliamps" :max="device.maxMilliamps" :aria-label="t('pages.strip.current.label')" />
            </ListRow>

            <!-- Power limit, with the slider under its label -->
            <ListRow :label="t('pages.strip.maxMilliamps.label')" :icon="FlashIcon" :value="`${device.maxMilliamps}mA`">
                <Slider :model-value="[device.maxMilliamps]" :min="device.minMaxMilliamps" :max="device.maxMaxMilliamps" :step="POWER_LIMIT_STEP" :thumb-label="t('pages.strip.maxMilliamps.label')" @update:model-value="handleMaxMilliampsChange" />
            </ListRow>
        </ListGroup>

        <!-- The device is dimming the LEDs -->
        <Alert v-if="isPowerLimited" color="yellow">
            <HugeiconsIcon :icon="FlashIcon" />
            <AlertTitle>{{ t('pages.strip.current.limitedTitle') }}</AlertTitle>
            <AlertDescription>{{ t('pages.strip.current.limitedDescription') }}</AlertDescription>
        </Alert>
    </div>
</template>