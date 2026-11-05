<script setup>
import { computed, onBeforeUnmount } from 'vue';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import PowerIcon from '@hugeicons/core-free-icons/PowerIcon';
import SunIcon from '@hugeicons/core-free-icons/Sun01Icon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { Card } from '@brumaombra/ui-vintage/card';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings, createSettingsSaver, refreshLiveState } from '~/composables/useDeviceState.js';
import { usePolling } from '~/composables/usePolling.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import MeterRow from '~/components/common/MeterRow.vue';
import SliderRow from '~/components/common/SliderRow.vue';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';
import MatrixPreview from '~/components/matrix/MatrixPreview.vue';
import ScenePicker from '~/components/scenes/ScenePicker.vue';

const STATE_REFRESH_MS = 2000; // How often the scene on screen and the current are read again
const { t } = useI18n();
const globalStore = useGlobalStore();
const saveBrightness = createSettingsSaver();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Brightness as a percentage, shown next to the slider
const brightnessPercentage = computed(() => Math.round((device.value.brightness / 255) * 100));

// Whether the estimated current is over the power limit (the device then dims the LEDs)
const isPowerLimited = computed(() => device.value.on && device.value.estimatedMilliamps >= device.value.maxMilliamps);

// Switch the matrix on or off right away
const handlePowerChange = isOn => {
    applyDeviceSettings({ on: isOn });
};

// Change the brightness (the device gets one request when the slider rests)
const handleBrightnessChange = brightness => {
    saveBrightness({ brightness });
};

// Pick the scene to show, or -1 to rotate them all
const handleSceneChange = scene => {
    applyDeviceSettings({ scene });
};

// Read again what the device changes by itself, while the page is visible
usePolling({ callback: refreshLiveState, intervalMs: STATE_REFRESH_MS });

// Send a brightness still waiting when leaving the page
onBeforeUnmount(() => saveBrightness.flush());

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <div class="mx-auto w-full max-w-2xl space-y-6">
        <!-- Title -->
        <PageHeader :title="t('pages.home.title')" size="medium" />

        <DeviceStateGate>
            <div class="space-y-6">
                <!-- Live preview of the matrix, on one card like the header of a profile -->
                <Card padding="none" class="p-2">
                    <MatrixPreview />
                </Card>

                <!-- Power and brightness -->
                <ListGroup :title="t('pages.home.displayTitle')" :hint="t('pages.home.brightness.note')">
                    <!-- Display on or off -->
                    <ListRow :label="t('pages.home.power.label')" :icon="PowerIcon" label-for="power">
                        <template #trailing>
                            <Switch id="power" :model-value="device.on" @update:model-value="handlePowerChange" />
                        </template>
                    </ListRow>

                    <!-- Brightness, with the slider under its label -->
                    <SliderRow id="brightness"
                        :model-value="device.brightness"
                        :label="t('pages.home.brightness.label')"
                        :icon="SunIcon"
                        :min="0"
                        :max="255"
                        :value-text="`${brightnessPercentage}%`"
                        @update:model-value="handleBrightnessChange" />
                </ListGroup>

                <!-- Current drawn by the LEDs, against the limit -->
                <div class="space-y-3">
                    <ListGroup :title="t('pages.home.current.title')">
                        <MeterRow :label="t('pages.home.current.label')"
                            :icon="FlashIcon"
                            :value="device.estimatedMilliamps"
                            :max="device.maxMilliamps"
                            :value-text="t('pages.home.current.value', { milliamps: device.estimatedMilliamps, limit: device.maxMilliamps })"
                            :end-label="t('pages.home.current.limit', { milliamps: device.maxMilliamps })" />
                    </ListGroup>

                    <!-- The device is dimming the LEDs -->
                    <Alert v-if="isPowerLimited" color="yellow">
                        <HugeiconsIcon :icon="FlashIcon" />
                        <AlertTitle>{{ t('pages.home.current.limitedTitle') }}</AlertTitle>
                        <AlertDescription>{{ t('pages.home.current.limitedDescription') }}</AlertDescription>
                    </Alert>
                </div>

                <!-- Scenes -->
                <ScenePicker :scenes="device.scenes" :model-value="device.scene" :current-scene="device.on ? device.currentScene : -1" @update:model-value="handleSceneChange" />
            </div>
        </DeviceStateGate>
    </div>
</template>
