<script setup>
import { computed, ref } from 'vue';
import Calendar03Icon from '@hugeicons/core-free-icons/Calendar03Icon';
import FireworksIcon from '@hugeicons/core-free-icons/FireworksIcon';
import PineTreeIcon from '@hugeicons/core-free-icons/PineTreeIcon';
import StarIcon from '@hugeicons/core-free-icons/StarIcon';
import { ListRow } from '@brumaombra/ui-vintage/list';
import { OptionPickerDialog } from '@brumaombra/ui-vintage/option-picker-dialog';
import { getHolidayName } from '~/composables/useHolidays.js';

// Props: the holidays of the device, the chosen one (-1 follows the calendar) and the one playing now
const props = defineProps({
    holidays: { type: Array, required: true },
    modelValue: { type: Number, default: -1 },
    activeHoliday: { type: Number, default: 0 }
});

// Emits
const emits = defineEmits(['update:modelValue']);

const { t, te } = useI18n();

// Icon of each holiday by id
const holidayIcons = {
    christmas: PineTreeIcon,
    'new-year': FireworksIcon
};

// Name of the holiday the calendar picked now
const calendarHolidayName = computed(() => {
    const holiday = props.holidays[props.activeHoliday];
    return holiday ? getHolidayName({ holiday, t, te }) : '';
});

// Options to pick from: Automatic (with the holiday of the calendar), then one per holiday
const options = computed(() => [
    { value: -1, label: t('pages.home.holidayAutomatic'), description: t('pages.home.holidayAutomaticNow', { holiday: calendarHolidayName.value }), icon: Calendar03Icon },
    ...props.holidays.map((holiday, index) => ({ value: index, label: getHolidayName({ holiday, t, te }), description: '', icon: holidayIcons[holiday.id] || StarIcon }))
]);

// Whether the holiday picker is open
const isPickerOpen = ref(false);

// The chosen option (Automatic when the chosen holiday isn't in the list)
const selectedOption = computed(() => options.value.find(option => option.value === props.modelValue) || options.value[0]);

// The text under the row: the holiday playing now while Automatic is chosen
const playingText = computed(() => props.modelValue < 0 ? t('pages.home.holidayAutomaticNow', { holiday: calendarHolidayName.value }) : '');

// Pick a holiday (a tap on the chosen one changes nothing)
const handleSelect = value => {
    if (value !== props.modelValue) emits('update:modelValue', value);
};
</script>

<template>
    <!-- The holiday of the strip: a row of the holiday group, with the chosen holiday, opening the picker -->
    <ListRow :label="t('pages.home.holidayLabel')" :value="selectedOption.label" :description="playingText" :icon="selectedOption.icon" action data-testid="holiday-row" @click="isPickerOpen = true" />

    <!-- The holidays to pick from: Automatic, then one per holiday -->
    <OptionPickerDialog v-model:open="isPickerOpen" :title="t('pages.home.holidayLabel')" :description="t('pages.home.holidaysDescription')" :options="options" :model-value="props.modelValue" @update:model-value="handleSelect" />
</template>
