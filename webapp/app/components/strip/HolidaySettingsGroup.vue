<script setup>
import { computed } from 'vue';
import HourglassIcon from '@hugeicons/core-free-icons/HourglassIcon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Switch } from '@brumaombra/ui-vintage/switch';
import { applyDeviceSettings } from '~/composables/useDeviceState.js';
import { getHolidayName } from '~/composables/useHolidays.js';
import { COUNTDOWN_DAYS } from '~/composables/useMessage.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import CustomMessageRow from '~/components/strip/CustomMessageRow.vue';
import MessageColorPicker from '~/components/strip/MessageColorPicker.vue';
import ScenePicker from '~/components/strip/ScenePicker.vue';

const { t, te } = useI18n();
const globalStore = useGlobalStore();

// Reactive device state
const device = computed(() => globalStore.value.device);

// The holiday playing (the chosen one, or the one of the calendar), whose settings the group shows
const holiday = computed(() => device.value.holidays[device.value.activeHoliday]);

// Pick the scene to show, or -1 to rotate them all
const handleSceneChange = scene => {
    applyDeviceSettings({ scene });
};

// Pick the color of the custom message
const handleMessageColorChange = messageColor => {
    applyDeviceSettings({ messageColor });
};

// Turn the Christmas countdown on or off right away
const handleCountdownChange = isOn => {
    applyDeviceSettings({ countdown: isOn });
};
</script>

<template>
    <!-- The settings of the holiday playing, in a group named after it: its scene, its custom message, and what only that holiday has -->
    <ListGroup v-if="holiday" :title="getHolidayName({ holiday, t, te })" :data-testid="`holiday-settings-${holiday.id}`">
        <!-- The scene, when there is more than one to pick from -->
        <ScenePicker v-if="holiday.scenes.length > 1" :scenes="holiday.scenes" :model-value="device.scene" :current-scene="device.on ? device.currentScene : -1" @update:model-value="handleSceneChange" />

        <!-- The custom message -->
        <CustomMessageRow :holiday-index="device.activeHoliday" />

        <!-- Custom message: the color of the text -->
        <MessageColorPicker v-if="holiday.id === 'custom'" :model-value="device.messageColor" @update:model-value="handleMessageColorChange" />

        <!-- Christmas: the countdown after the greeting -->
        <ListRow v-if="holiday.id === 'christmas'" :label="t('pages.strip.message.countdown.label')" :description="t('pages.strip.message.countdown.note', { days: COUNTDOWN_DAYS })" :icon="HourglassIcon" label-for="countdown">
            <template #trailing>
                <Switch id="countdown" :model-value="device.countdown" @update:model-value="handleCountdownChange" />
            </template>
        </ListRow>
    </ListGroup>
</template>