<script setup>
import { computed } from 'vue';
import { Card } from '@brumaombra/ui-vintage/card';
import { ListGroup } from '@brumaombra/ui-vintage/list';
import { applyDeviceSettings, refreshLiveState } from '~/composables/useDeviceState.js';
import { usePolling } from '~/composables/usePolling.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';
import DisplayGroup from '~/components/strip/DisplayGroup.vue';
import HolidayPicker from '~/components/strip/HolidayPicker.vue';
import HolidaySettingsGroup from '~/components/strip/HolidaySettingsGroup.vue';
import MatrixPreview from '~/components/strip/MatrixPreview.vue';
import PowerGroup from '~/components/strip/PowerGroup.vue';

const STATE_REFRESH_MS = 2000; // How often the holiday and scene on screen and the current are read again
const { t } = useI18n();
const globalStore = useGlobalStore();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Pick the holiday to show, or -1 to follow the calendar
const handleHolidayChange = holiday => {
    applyDeviceSettings({ holiday });
};

// Read again what the device changes by itself, while the page is visible
usePolling({ callback: refreshLiveState, intervalMs: STATE_REFRESH_MS });

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <GizmoPage :title="t('pages.strip.title')">
        <DeviceStateGate>
            <div class="space-y-6">
                <!-- Live preview of the matrix, on one card like the header of a profile -->
                <Card padding="none" class="p-2">
                    <MatrixPreview />
                </Card>

                <!-- On or off, and the brightness -->
                <DisplayGroup />

                <!-- The holiday playing: the calendar, or one picked -->
                <ListGroup v-if="device.holidays.length" :title="t('pages.strip.holidaysTitle')" :hint="t('pages.strip.holidaysDescription')">
                    <HolidayPicker :holidays="device.holidays" :model-value="device.holiday" :active-holiday="device.activeHoliday" @update:model-value="handleHolidayChange" />
                </ListGroup>

                <!-- The settings of that holiday -->
                <HolidaySettingsGroup />

                <!-- The current drawn and the power limit -->
                <PowerGroup />
            </div>
        </DeviceStateGate>
    </GizmoPage>
</template>