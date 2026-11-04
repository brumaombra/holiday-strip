<script setup>
import { computed, onBeforeUnmount } from 'vue';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import PowerIcon from '@hugeicons/core-free-icons/PowerIcon';
import SparklesIcon from '@hugeicons/core-free-icons/SparklesIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { PageHeader } from '@brumaombra/ui-vintage/page-header';
import { ProgressComponent } from '@brumaombra/ui-vintage/progress-component';
import { SectionTitle } from '@brumaombra/ui-vintage/section-title';
import { SliderFormComponent } from '@brumaombra/ui-vintage/slider-form-component';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings, createSettingsSaver, refreshLiveState } from '~/composables/useDeviceState.js';
import { usePolling } from '~/composables/usePolling.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
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
                <!-- Live preview of the matrix -->
                <section>
                    <SectionTitle :title="t('pages.home.previewTitle')" :icon="SparklesIcon" />
                    <div class="rounded border border-border bg-card p-2">
                        <MatrixPreview />
                    </div>
                </section>

                <!-- Power and brightness -->
                <div class="space-y-4">
                    <ListGroup>
                        <ListRow :label="t('pages.home.power.label')" :icon="PowerIcon" label-for="power">
                            <template #trailing>
                                <Switch id="power" :model-value="device.on" @update:model-value="handlePowerChange" />
                            </template>
                        </ListRow>
                    </ListGroup>
                    <SliderFormComponent id="brightness"
                        :model-value="device.brightness"
                        :label="t('pages.home.brightness.label')"
                        :description="t('pages.home.brightness.note')"
                        :min="0"
                        :max="255"
                        :value-text="`${brightnessPercentage}%`"
                        @update:model-value="handleBrightnessChange" />
                </div>

                <!-- Current drawn by the LEDs, against the limit -->
                <div class="space-y-3">
                    <ProgressComponent :title="t('pages.home.current.title')"
                        :value="device.estimatedMilliamps"
                        :max="device.maxMilliamps"
                        :bottom-left-label="t('pages.home.current.estimated', { milliamps: device.estimatedMilliamps })"
                        :bottom-right-label="t('pages.home.current.limit', { milliamps: device.maxMilliamps })" />
                    <Alert v-if="isPowerLimited" color="yellow">
                        <HugeiconsIcon :icon="FlashIcon" />
                        <AlertTitle>{{ t('pages.home.current.limitedTitle') }}</AlertTitle>
                        <AlertDescription>{{ t('pages.home.current.limitedDescription') }}</AlertDescription>
                    </Alert>
                </div>

                <!-- Scenes -->
                <section>
                    <SectionTitle :title="t('pages.home.scenesTitle')" :description="t('pages.home.scenesDescription')" />
                    <ScenePicker :scenes="device.scenes" :model-value="device.scene" :current-scene="device.on ? device.currentScene : -1" @update:model-value="handleSceneChange" />
                </section>
            </div>
        </DeviceStateGate>
    </div>
</template>
