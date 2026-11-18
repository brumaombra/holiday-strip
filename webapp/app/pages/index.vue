<script setup>
import { computed, onBeforeUnmount } from 'vue';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import HourglassIcon from '@hugeicons/core-free-icons/HourglassIcon';
import PowerIcon from '@hugeicons/core-free-icons/PowerIcon';
import SunIcon from '@hugeicons/core-free-icons/Sun01Icon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { Card } from '@brumaombra/ui-vintage/card';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Progress } from '@brumaombra/ui-vintage/progress';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings, createSettingsSaver, refreshLiveState } from '~/composables/useDeviceState.js';
import { COUNTDOWN_DAYS } from '~/composables/useMessage.js';
import { usePolling } from '~/composables/usePolling.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';
import MatrixPreview from '~/components/matrix/MatrixPreview.vue';
import HolidayPicker from '~/components/holidays/HolidayPicker.vue';
import CustomMessageRow from '~/components/message/CustomMessageRow.vue';
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
const handleBrightnessChange = ([brightness]) => {
    saveBrightness({ brightness });
};

// The holiday whose settings the group shows: the chosen one, or the one playing now while the calendar picks it
const shownHoliday = computed(() => device.value.holiday >= 0 ? device.value.holiday : device.value.activeHoliday);
const shownHolidayId = computed(() => device.value.holidays[shownHoliday.value]?.id);

// Scenes of the holiday playing now (none until the device sends its holidays)
const activeScenes = computed(() => device.value.holidays[device.value.activeHoliday]?.scenes || []);

// Pick the holiday to show, or -1 to follow the calendar
const handleHolidayChange = holiday => {
    applyDeviceSettings({ holiday });
};

// Pick the scene to show, or -1 to rotate them all
const handleSceneChange = scene => {
    applyDeviceSettings({ scene });
};

// Turn the Christmas countdown on or off right away
const handleCountdownChange = isOn => {
    applyDeviceSettings({ countdown: isOn });
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
    <GizmoPage :title="t('pages.home.title')">
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
                    <ListRow :label="t('pages.home.brightness.label')" :icon="SunIcon" :value="`${brightnessPercentage}%`">
                        <Slider :model-value="[device.brightness]" :min="0" :max="255" :thumb-label="t('pages.home.brightness.label')" @update:model-value="handleBrightnessChange" />
                    </ListRow>
                </ListGroup>

                <!-- Current drawn by the LEDs, against the limit -->
                <div class="space-y-3">
                    <ListGroup :title="t('pages.home.current.title')">
                        <ListRow :label="t('pages.home.current.label')" :description="t('pages.home.current.limit', { milliamps: device.maxMilliamps })" :icon="FlashIcon" :value="t('pages.home.current.value', { milliamps: device.estimatedMilliamps, limit: device.maxMilliamps })">
                            <Progress :model-value="device.estimatedMilliamps" :max="device.maxMilliamps" :aria-label="t('pages.home.current.label')" />
                        </ListRow>
                    </ListGroup>

                    <!-- The device is dimming the LEDs -->
                    <Alert v-if="isPowerLimited" color="yellow">
                        <HugeiconsIcon :icon="FlashIcon" />
                        <AlertTitle>{{ t('pages.home.current.limitedTitle') }}</AlertTitle>
                        <AlertDescription>{{ t('pages.home.current.limitedDescription') }}</AlertDescription>
                    </Alert>
                </div>

                <!-- The holiday, then the settings of the one shown: its scene, its custom message, and what only that holiday has -->
                <ListGroup v-if="device.holidays.length" :title="t('pages.home.holidaysTitle')" :hint="t('pages.home.holidaysDescription')">
                    <HolidayPicker :holidays="device.holidays" :model-value="device.holiday" :active-holiday="device.activeHoliday" @update:model-value="handleHolidayChange" />
                    <ScenePicker :scenes="activeScenes" :model-value="device.scene" :current-scene="device.on ? device.currentScene : -1" @update:model-value="handleSceneChange" />
                    <CustomMessageRow :holiday-index="shownHoliday" />

                    <!-- Christmas: the countdown after the greeting -->
                    <ListRow v-if="shownHolidayId === 'christmas'" :label="t('pages.message.countdown.label')" :description="t('pages.message.countdown.note', { days: COUNTDOWN_DAYS })" :icon="HourglassIcon" label-for="countdown">
                        <template #trailing>
                            <Switch id="countdown" :model-value="device.countdown" @update:model-value="handleCountdownChange" />
                        </template>
                    </ListRow>
                </ListGroup>
            </div>
        </DeviceStateGate>
    </GizmoPage>
</template>
