<script setup>
import { computed, onBeforeUnmount } from 'vue';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { createSettingsSaver } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';

const POWER_LIMIT_STEP = 100; // Milliamps
const { t } = useI18n();
const globalStore = useGlobalStore();
const saveMaxMilliamps = createSettingsSaver();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Change the power limit (the device gets one request when the slider rests)
const handleMaxMilliampsChange = ([maxMilliamps]) => {
    saveMaxMilliamps({ maxMilliamps });
};

// Send a power limit still waiting when leaving the page
onBeforeUnmount(() => saveMaxMilliamps.flush());

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <GizmoPage :title="t('pages.device.title')">
        <div class="space-y-6">
            <!-- Power supply: the strip's own setting of the device -->
            <DeviceStateGate>
                <ListGroup :title="t('pages.device.powerTitle')" :hint="t('pages.device.maxMilliamps.note')">
                    <ListRow :label="t('pages.device.maxMilliamps.label')" :icon="FlashIcon" :value="`${device.maxMilliamps}mA`">
                        <Slider :model-value="[device.maxMilliamps]" :min="device.minMaxMilliamps" :max="device.maxMaxMilliamps" :step="POWER_LIMIT_STEP" :thumb-label="t('pages.device.maxMilliamps.label')" @update:model-value="handleMaxMilliampsChange" />
                    </ListRow>
                </ListGroup>
            </DeviceStateGate>

            <!-- Device info, network, restart and forget network (from the gizmo-common layer, it reads the device by itself). The language is picked in the top bar, like in every OpenGizmo app -->
            <GizmoDeviceCard />
        </div>
    </GizmoPage>
</template>