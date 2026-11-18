<script setup>
import { computed } from 'vue';
import { ListGroup } from '@brumaombra/ui-vintage/list';
import { applyDeviceSettings } from '~/composables/useDeviceState.js';
import { getHolidayName } from '~/composables/useHolidays.js';
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
</script>

<template>
    <!-- The settings of the holiday playing, in a group named after it: its scene, its custom message, and what only that holiday has (the text color of the custom message) -->
    <ListGroup v-if="holiday" :title="getHolidayName({ holiday, t, te })" :data-testid="`holiday-settings-${holiday.id}`">
        <!-- The scene, when there is more than one to pick from -->
        <ScenePicker v-if="holiday.scenes.length > 1" :scenes="holiday.scenes" :model-value="device.scene" :current-scene="device.on ? device.currentScene : -1" @update:model-value="handleSceneChange" />

        <!-- The custom message -->
        <CustomMessageRow :holiday-index="device.activeHoliday" />

        <!-- Custom message: the color of the text -->
        <MessageColorPicker v-if="holiday.id === 'custom'" :model-value="device.messageColor" @update:model-value="handleMessageColorChange" />

    </ListGroup>
</template>