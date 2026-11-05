<script setup>
import { computed } from 'vue';
import HourglassIcon from '@hugeicons/core-free-icons/HourglassIcon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings } from '~/composables/useDeviceState.js';
import { COUNTDOWN_DAYS } from '~/composables/useMessage.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';
import MessageForm from '~/components/message/MessageForm.vue';

const { t } = useI18n();
const globalStore = useGlobalStore();

// Reactive device state
const device = computed(() => globalStore.value.device);

// Turn the countdown on or off right away
const handleCountdownChange = isOn => {
    applyDeviceSettings({ countdown: isOn });
};

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <GizmoPage :title="t('pages.message.title')">
        <DeviceStateGate>
            <div class="space-y-6">
                <!-- Scrolling message -->
                <MessageForm />

                <!-- Countdown to Christmas -->
                <ListGroup :title="t('pages.message.countdown.title')" :hint="t('pages.message.countdown.note', { days: COUNTDOWN_DAYS })">
                    <ListRow :label="t('pages.message.countdown.label')" :icon="HourglassIcon" label-for="countdown">
                        <template #trailing>
                            <Switch id="countdown" :model-value="device.countdown" @update:model-value="handleCountdownChange" />
                        </template>
                    </ListRow>
                </ListGroup>
            </div>
        </DeviceStateGate>
    </GizmoPage>
</template>
