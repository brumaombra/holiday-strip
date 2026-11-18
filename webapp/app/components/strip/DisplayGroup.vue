<script setup>
import { computed, onBeforeUnmount } from 'vue';
import PowerIcon from '@hugeicons/core-free-icons/PowerIcon';
import SunIcon from '@hugeicons/core-free-icons/Sun01Icon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings, createSettingsSaver } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const { t } = useI18n();
const globalStore = useGlobalStore();
const saveBrightness = createSettingsSaver();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Brightness as a percentage, shown next to the slider
const brightnessPercentage = computed(() => Math.round((device.value.brightness / 255) * 100));

// Switch the matrix on or off right away
const handlePowerChange = isOn => {
    applyDeviceSettings({ on: isOn });
};

// Change the brightness (the device gets one request when the slider rests)
const handleBrightnessChange = ([brightness]) => {
    saveBrightness({ brightness });
};

// Send a brightness still waiting when leaving the page
onBeforeUnmount(() => saveBrightness.flush());
</script>

<template>
    <!-- The display: on or off, and its brightness -->
    <ListGroup :title="t('pages.strip.displayTitle')" :hint="t('pages.strip.brightness.note')">
        <!-- Display on or off -->
        <ListRow :label="t('pages.strip.power.label')" :icon="PowerIcon" label-for="power">
            <template #trailing>
                <Switch id="power" :model-value="device.on" @update:model-value="handlePowerChange" />
            </template>
        </ListRow>

        <!-- Brightness, with the slider under its label -->
        <ListRow :label="t('pages.strip.brightness.label')" :icon="SunIcon" :value="`${brightnessPercentage}%`">
            <Slider :model-value="[device.brightness]" :min="0" :max="255" :thumb-label="t('pages.strip.brightness.label')" @update:model-value="handleBrightnessChange" />
        </ListRow>
    </ListGroup>
</template>