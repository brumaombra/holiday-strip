<script setup>
import { computed, ref } from 'vue';
import MessageSquareTextIcon from '@hugeicons/core-free-icons/MessageSquareTextIcon';
import { ListRow } from '@brumaombra/ui-vintage/list';
import { ResponsiveDialog } from '@brumaombra/ui-vintage/responsive-dialog';
import { getHolidayName } from '~/composables/useHolidays.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import MessageEditor from '~/components/message/MessageEditor.vue';

// Props: the index of the holiday whose custom message is shown
const props = defineProps({
    holidayIndex: { type: Number, required: true }
});

const { t, te } = useI18n();
const globalStore = useGlobalStore();

// Reactive device state
const device = computed(() => globalStore.value.device);

// The holiday of this message, and its message on the device
const holiday = computed(() => device.value.holidays[props.holidayIndex]);
const message = computed(() => device.value.messages[props.holidayIndex] ?? '');

// Whether the editor is open
const isEditorOpen = ref(false);
</script>

<template>
    <!-- The custom message of the holiday: a row of the holiday group with the message under its label, opening the editor -->
    <ListRow :label="t('pages.message.label')" :description="message" :icon="MessageSquareTextIcon" action :data-testid="`message-row-${holiday.id}`" @click="isEditorOpen = true" />

    <!-- The editor, a sheet on phones (it starts from the message of the device every time it opens) -->
    <ResponsiveDialog v-model:open="isEditorOpen" :title="t('pages.message.label')" :description="getHolidayName({ holiday, t, te })">
        <MessageEditor v-if="isEditorOpen" :key="holiday.id" :holiday-index="props.holidayIndex" @saved="isEditorOpen = false" />
    </ResponsiveDialog>
</template>